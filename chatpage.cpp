#include "chatpage.h"
#include "ui_chatpage.h"
#include <QStyleOption>
#include <QPainter>
#include <QLabel>
#include "usermgr.h"
#include "chatitembase.h"
#include "textbubble.h"
#include "tcpmgr.h"
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTime>
ChatPage::ChatPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatPage)
{
    ui->setupUi(this);

    ui->receive_btn->SetState("normal","hover","press");
    ui->send_btn->SetState("normal","hover","press");

    ui->emo_lb->SetState("normal","hover","press","normal","hover","press");
    ui->file_lb->SetState("normal","hover","press","normal","hover","press");
    connect(ui->send_btn, &ClickedBtn::clicked, this, &ChatPage::on_send_btn_clicked);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_send_msg_rsp, this, &ChatPage::slot_send_msg_rsp);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_recv_msg,this, &ChatPage::slot_recv_msg);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_load_msg_rsp,this, &ChatPage::slot_load_msg_rsp);
    connect(ui->chat_data_list, &ChatView::sig_load_more, this, &ChatPage::slot_load_more);
}

ChatPage::~ChatPage()
{
    delete ui;
}

void ChatPage::SetTitle(const QString &title)
{
    ui->title_lb->setText(title);
}

void ChatPage::SetSession(int thread_id, int peer_uid, const QString &name, const QString &icon)
{
    if(peer_uid != 0 && peer_uid == _peer_id){
        ui->title_lb->setText(name);
        if(!icon.isEmpty()){
            _peer_icon = icon;
        }
        return;
    }
    _thread_id = thread_id;
    _peer_id = peer_uid;
    _peer_icon = icon;

    ui->title_lb->setText(name);
    ui->chat_data_list->removeAllItem();

    if(_thread_id > 0) {
        QJsonObject req;
        req["thread_id"] = _thread_id;
        req["count"] = 20;
        emit TcpMgr::GetInstance()->sig_send_data(
            ReqId::ID_LOAD_CHAT_MSG_REQ, QString(QJsonDocument(req).toJson(QJsonDocument::Compact)));
    }

    _oldest_msg_id = 0;
    _loading_history = false;

    if(_thread_id == 0) {
        auto *tip = new QLabel(tr("还没有聊天记录, 发送第一条消息开始聊天吧"));
        tip->setAlignment(Qt::AlignCenter);
        tip->setStyleSheet("color: gray;");
        ui->chat_data_list->appendChatItem(tip);
    }
}

void ChatPage::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ChatPage::on_send_btn_clicked()
{
    const QString text = ui->chat_edit->toPlainText().trimmed();
    if (text.isEmpty()) {
        return;
    }

    if(_peer_id == 0) {
        QMessageBox::information(this, tr("提示"),tr("还没选中聊天对象"));
        return;
    }

    QJsonObject req;
    req["to_uid"] = _peer_id;
    req["content"] = text;
    req["msg_type"] = 1;
    emit TcpMgr::GetInstance()->sig_send_data(
        ReqId::ID_TEXT_CHAT_MSG_REQ,             // = 1017
        QString(QJsonDocument(req).toJson(QJsonDocument::Compact)));

    _last_sent_text = text;      // 记下要显示的文本

    ui->chat_edit->clear();
}

void ChatPage::slot_send_msg_rsp(const QJsonObject &json)
{
    const int err = json["error"].toInt();
    if(err != 0) {
        QMessageBox::information(this, tr("提示"),tr("发送失败,错误码%1").arg(err));
        return;
    }

    // 服务端会带回 thread_id: 第一次聊天时它会从 0 变成真值
    if(_thread_id == 0 && json.contains("thread_id")) {
        _thread_id = json["thread_id"].toInt();
        if(_thread_id > 0) {
            emit sig_thread_id_updated(_peer_id, _thread_id);
        }
    }

    // 画自己的绿色气泡
    auto *item = new ChatItemBase(ChatRole::Self);
    item->setUserName(UserMgr::GetInstance()->GetName().isEmpty()
                          ? tr("我") : UserMgr::GetInstance()->GetName());
    QString my_icon = UserMgr::GetInstance()->GetIcon();
    item->setUserIcon(QPixmap(my_icon.isEmpty() ? QStringLiteral(":/res/head_1.jpg") : my_icon));
    auto *bubble = new TextBubble(ChatRole::Self,
                                  json.contains("content") ? json["content"].toString()
                                                           : _last_sent_text);
    item->setWidget(bubble);
    ui->chat_data_list->appendChatItem(item);
    emit sig_msg_sent(_peer_id, json.contains("content") ? json["content"].toString() : _last_sent_text);
}

void ChatPage::slot_recv_msg(const QJsonObject &json)
{
    const int from_uid  = json["from_uid"].toInt();
    const int thread_id = json["thread_id"].toInt();
    const QString text = json.contains("content") ? json["content"].toString() : json["msg_content"].toString();
    const QString name  = json.contains("name") ? json["name"].toString()
                                               : json["nick"].toString();

    if(_peer_id != from_uid) {
        return;
    }

    if(_thread_id == 0 && thread_id > 0) {
        _thread_id = thread_id;
        emit sig_thread_id_updated(_peer_id, _thread_id);
    }

    auto *item = new ChatItemBase(ChatRole::Other);
    item->setUserName(name.isEmpty()? ui->title_lb->text() : name);

    QPixmap head(_peer_icon.isEmpty() ? QStringLiteral(":/res/head_1.jpg") : _peer_icon);
    item->setUserIcon(head);

    auto *bubble = new TextBubble(ChatRole::Other, text);
    item->setWidget(bubble);
    ui->chat_data_list->appendChatItem(item);
}

void ChatPage::slot_load_msg_rsp(const QJsonObject &json)
{
    const bool is_more = _b_prepend;
    _b_prepend = false;
    _loading_history = false;
    if(json["error"].toInt() != 0) {
        return;
    }

    const QJsonArray msgs = json.contains("messages") ? json["messages"].toArray() : json["msgs"].toArray();

    if (msgs.isEmpty()) {
        return;
    }

    const int my_uid = UserMgr::GetInstance()->GetUid();
    QList<QWidget*> items;

    QString my_name = UserMgr::GetInstance()->GetName();
    if(my_name.isEmpty()) {
        my_name = tr("我");
    }

    QString my_icon = UserMgr::GetInstance()->GetIcon();
    if(my_icon.isEmpty()) {
        my_icon = QStringLiteral(":/res/head_1.jpg");
    }

    for(const auto& v:msgs) {
        const QJsonObject o = v.toObject();
        const int from_uid = o["from_uid"].toInt();

        // 服务端已按时间正序返回(DAO 里 reverse 过) -> 按顺序 append 即可
        const bool is_self = (from_uid == my_uid);

        //  历史里"我"那条也要用真名真头像, 和实时路径保持一致
        auto *item = new ChatItemBase(is_self ? ChatRole::Self : ChatRole::Other);
        item->setUserName(is_self ? my_name : ui->title_lb->text());
        item->setUserIcon(QPixmap(is_self ? my_icon
                                          : (_peer_icon.isEmpty() ? QStringLiteral(":/res/head_1.jpg") : _peer_icon)));
        item->setWidget(new TextBubble(is_self ? ChatRole::Self : ChatRole::Other,
                                       o["content"].toString()));
        items.append(item);
    }

    if (is_more) {
        ui->chat_data_list->prependChatItems(items);
    }else{
        for (auto* w : items) {
            ui->chat_data_list->appendChatItem(w);
        }
        QScrollBar* bar = ui->chat_data_list->GetScrollBar();
        if (bar != nullptr) {
            bar->setValue(bar->maximum());
        }
    }

    _oldest_msg_id = json.contains("next_before_id")
                     ?  json["next_before_id"].toVariant().toLongLong()
                     : 0;

    //  打开会话加载历史时: 用最后一条刷新左栏的"最后一条消息"。
    //   必须把这条消息【自己的时间】(create_time)一起发出去:
    //     以前这里只发内容, ChatDialog 只好拿"现在"当时间戳 ->
    //     表现出来就是"时间 = 我最后一次点进这个会话的时间"(你报的那个 bug)。
    //     服务端 1030 回包里每条消息都带 create_time(见 LogicSystem.cpp:643)。
    if (!is_more && !msgs.isEmpty()) {
        const QJsonObject last = msgs.last().toObject();
        emit sig_msg_sent(_peer_id,
                          last["content"].toString(),
                          last["create_time"].toString());
    }
}

void ChatPage::slot_load_more()
{
    if (_thread_id <= 0 || _oldest_msg_id <= 0 || _loading_history){
        return;
    }
    _loading_history = true;
    _b_prepend = true;
    QJsonObject req;
    req["thread_id"] = _thread_id;
    req["count"] = 20;
    req["before_msg_id"] = _oldest_msg_id;
    emit TcpMgr::GetInstance()->sig_send_data(
        ReqId::ID_LOAD_CHAT_MSG_REQ,
        QString(QJsonDocument(req).toJson(QJsonDocument::Compact)));
}


