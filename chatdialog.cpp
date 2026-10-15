#include "chatdialog.h"
#include "ui_chatdialog.h"
#include <QTime>
#include <QDateTime>
#include <QAction>
#include <QIcon>
#include <QJsonArray>
#include <QJsonObject>
#include <QListWidgetItem>
#include <QPixmap>
#include <QRandomGenerator>
#include <QDebug>
#include "chatuserlist.h"
#include "chatuserwid.h"
#include "customizeedit.h"
#include "tcpmgr.h"
#include "applyfriendpage.h"
#include "usermgr.h"
#include "userdata.h"
#include "contactuserlist.h"
#include "friendinfopage.h"
// ============================================================
// 把服务端给的时间(MySQL DATETIME 字符串, 形如 "2026-10-14 23:24:36")
// 变成左栏右上角那两三个字:
//     今天   -> "HH:mm"
//     昨天   -> "昨天"
//     更早   -> "M月d日"
//   空 / 解析不出来 -> ""(调用方自己用"现在"兜底)
// ============================================================
static QString FormatChatTime(const QString& server_time)
{
    if (server_time.isEmpty()) {
        return QString();
    }

    const QDateTime dt = QDateTime::fromString(server_time, "yyyy-MM-dd HH:mm:ss");
    if (!dt.isValid()) {
        return server_time;
    }

    const QDate today = QDate::currentDate();
    if (dt.date() == today) {
        return dt.toString("HH:mm");
    }
    if (dt.date() == today.addDays(-1)) {
        return QObject::tr("昨天");
    }
    return dt.toString(QStringLiteral("M月d日"));
}

ChatDialog::ChatDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChatDialog)
    , _mode(ChatUIMode::ChatMode)
    , _state(ChatUIMode::ChatMode)
    , _b_loading(false)
{
    ui->setupUi(this);

    ui->add_btn->SetState("normal", "hover", "press");
    //   六态: normal / hover / press / selected / selected_hover / selected_press
    ui->side_chat_lb->SetState("normal","hover","press","selected","selected_hover","selected_pressed");
    ui->side_contact_lb->SetState("normal","hover","press","selected","selected_hover","selected_pressed");
    ui->side_apply_lb->SetState("normal","hover","press","selected","selected_hover","selected_pressed");

    // 登记进"互斥组"
    AddLBGroup(ui->side_chat_lb);
    AddLBGroup(ui->side_contact_lb);
    AddLBGroup(ui->side_apply_lb);

    connect(ui->side_chat_lb,    &StateWidget::clicked, this, &ChatDialog::slot_side_chat);
    connect(ui->side_contact_lb, &StateWidget::clicked, this, &ChatDialog::slot_side_contact);
    connect(ui->side_apply_lb,   &StateWidget::clicked, this, &ChatDialog::slot_side_apply);

    // ---- 搜索框 ----
    initSearchEdit();

    // ---- "+" 按钮: 展开/收起搜索 ----
    connect(ui->add_btn, &QPushButton::clicked, this, &ChatDialog::slot_add_btn_clicked);

    // ---- 列表滚到底 -> 加载更多 ----
    connect(ui->chat_user_list, &ChatUserList::sig_loading_chat_user,
            this, &ChatDialog::slot_loading_chat_user);

    initSelfInfo();

    // 默认: 显示聊天列表, 藏起搜索列表
    ShowSearch(false);

    ui->search_list->SetSearchEdit(ui->search_edit);
    connect(ui->search_edit, &QLineEdit::textChanged, this, &ChatDialog::slot_text_changed);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_friend_apply,this,&ChatDialog::slot_friend_apply);

    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_friend_auth,
            this, &ChatDialog::slot_friend_auth);

    qApp->installEventFilter(this);

    connect(ui->chat_user_list, &QListWidget::itemClicked,this,&ChatDialog::slot_chat_user_clicked);

    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_recv_msg, this, &ChatDialog::slot_recv_msg);

    connect(ui->friend_apply_page, &ApplyFriendPage::sig_new_friend, this, &ChatDialog::slot_new_friend);

    connect(ui->chat_page, &ChatPage::sig_msg_sent, this, &ChatDialog::slot_msg_sent);

    //   ChatPage 第一次发消息后才知道真正的 thread_id, 它会 emit sig_thread_id_updated(见 chatpage.cpp:123)
    //   这里没人接 -> 左边格子里存的 thread_id 永远是 0
    //   -> 切到别的好友再点回来, SetSession(0) -> 不拉历史, 聊天记录整段"消失"
    connect(ui->chat_page, &ChatPage::sig_thread_id_updated,
            this, &ChatDialog::updateChatUserThreadId);

    // ---- 联系人页 / 好友资料页(我们自己补的) ----
    //   点联系人列表的一行 -> 右侧大页面切到"好友资料页"
    connect(ui->con_user_list, &ContactUserList::sig_switch_friend_info_page,
            this, &ChatDialog::slot_friend_info_page);

    //   资料页上点"发消息" -> 切回会话页, 并替用户把这个人的会话行点开
    connect(ui->friend_info_page, &FriendInfoPage::sig_jump_chat_item,
            this, &ChatDialog::slot_jump_chat_item_from_infopage);

}

ChatDialog::~ChatDialog()
{
    qApp->removeEventFilter(this);
    delete ui;
}

void ChatDialog::initSearchEdit()
{
    // 前导(左边): 放大镜
    QAction *searchAction = new QAction(ui->search_edit);
    searchAction->setIcon(QIcon(":/res/search.png"));
    ui->search_edit->addAction(searchAction, QLineEdit::LeadingPosition);
    ui->search_edit->setPlaceholderText(QStringLiteral("搜索"));

    // 尾部(右边): "×"
    //   初始给一张【透明图】= 相当于看不见; 有文字时才换成真的 ×。
    //   为什么用"换图标"而不是 show/hide: 输入框右边的宽度不会跳动(教程的做法)
    QAction *clearAction = new QAction(ui->search_edit);
    clearAction->setIcon(QIcon(":/res/close_transparent.png"));
    ui->search_edit->addAction(clearAction, QLineEdit::TrailingPosition);

    connect(ui->search_edit, &QLineEdit::textChanged, [clearAction](const QString &text) {
        clearAction->setIcon(QIcon(text.isEmpty() ? ":/res/close_transparent.png"
                                                  : ":/res/close_search.png"));
    });

    // 点"×": 清空文字 + 图标换回透明
    connect(clearAction, &QAction::triggered, [this, clearAction]() {
        ui->search_edit->clear();
        clearAction->setIcon(QIcon(":/res/close_transparent.png"));
        ui->search_edit->clearFocus();
        // 教程在这里把"收起搜索"注释掉了 —— 点 × 只清空文字, 不退出搜索模式
        // ShowSearch(false);
    });

    ui->search_edit->SetMaxLength(15);   //  按字节限长(自定义方法, 不会把汉字切碎)
}

// ------------------------------------------------------------
// 左下角显示自己的头像
//   能直接取到的原因: tcpmgr 处理 1006 时是先 SetUid/SetName, 再 emit sig_switch_chatdlg,
//   所以这个对话框被 new 出来的时候, UserMgr 里已经有值了
// ------------------------------------------------------------
void ChatDialog::initSelfInfo()
{
    QPixmap head(":/res/head_1.jpg");
    if (!head.isNull()) {
        ui->side_head_lb->setPixmap(head.scaled(ui->side_head_lb->size(),
                                                Qt::KeepAspectRatio,
                                                Qt::SmoothTransformation));
    }
}

// ------------------------------------------------------------
// 切换"聊天列表 / 搜索列表"(教程第 19 集)
//   _state 记"上一次的稳定模式(聊天/联系人)", _mode 记"现在在哪儿",
//   所以从搜索退出来的时候, 知道该回到哪一页。
// ------------------------------------------------------------
void ChatDialog::ShowSearch(bool bsearch)
{
    if (bsearch) {
        ui->chat_user_list->hide();
        ui->con_user_list->hide();      //  联系人列表也要一起收起
        ui->search_list->show();
        _mode = ChatUIMode::SearchMode;
    } else {
        ui->search_list->hide();

        //  会话列表 / 联系人列表 占的是同一块左栏(不是 QStackedWidget),
        //   只能"互斥显示": 谁该显示谁显示, 另一个 hide()。
        //   两个都 show 的话会上下挤在一起。
        if (_state == ChatUIMode::ContactMode) {
            ui->chat_user_list->hide();
            ui->con_user_list->show();
        } else {
            ui->con_user_list->hide();
            ui->chat_user_list->show();
        }
        _mode = _state;
    }
}

void ChatDialog::addChatUserItem(const QString& name, const QString& head,
                                 const QString& msg, int uid, int thread_id)
{
    // 拦截非法uid，不产生孤儿项
    if (uid <= 0) {
        qWarning() << "addChatUserItem: invalid uid" << uid;
        return;
    }

    // 去重检查：已存在则直接返回（如需更新信息可在这里调用更新接口）
    if (_user_items.contains(uid)) {
        return;
    }

    auto *chat_user_wid = new ChatUserWid();                  // ① 造"行控件"
    chat_user_wid->SetInfo(name, head, msg);                  // ② 填内容

    // ②' 这一行如果是"因为刚收到 / 刚发出消息"才建出来的, 把那一刻记下的时间贴上
    //     (SetLastMsg 第二个参数为空时不动时间标签; 重建列表时这里也能把时间贴回来)
    chat_user_wid->SetLastMsg(msg, _last_msg_time.value(uid));

    auto *item = new QListWidgetItem;                         // ③ 造"格子"
    item->setSizeHint(chat_user_wid->sizeHint());             // ④ 告诉格子多高(不写会被挤扁)

    item->setData(Qt::UserRole, uid);                     // uid 挂在格子上, 以后点击用
    item->setData(Qt::UserRole+1, thread_id);
    item->setData(Qt::UserRole+2, name);

    ui->chat_user_list->addItem(item);                        // ⑤ 格子进列表
    ui->chat_user_list->setItemWidget(item, chat_user_wid);   // ⑥ 控件贴进格子
    // _user_items.insert(uid, chat_user_wid);
    _user_items[uid] = chat_user_wid;
}

void ChatDialog::updateChatUserThreadId(int uid, int thread_id)
{
    if(uid <= 0 || thread_id <= 0) {
        return;
    }

    for(int i = 0; i < ui->chat_user_list->count();++i){
        QListWidgetItem* lw = ui->chat_user_list->item(i);
        if(lw != nullptr && lw->data(Qt::UserRole).toInt() == uid){
            lw->setData(Qt::UserRole + 1, thread_id);
            return;
        }
    }


}
// 登记: 把按钮放进列表, 以后"清别人"时用
void ChatDialog::AddLBGroup(StateWidget *lb)
{
    _lb_list.push_back(lb);
}
// 互斥的核心: 遍历列表, 把"除了刚点的这个"以外全部恢复未选中
void ChatDialog::ClearLabelState(StateWidget *lb)
{
    for(auto &ele : _lb_list) {
        if(ele == lb) continue;
        ele->ClearState();
    }
}

void ChatDialog::handleGlobalMousePress(QMouseEvent *event)
{
    // 只有"当前正在搜索"时才需要处理
    if (_mode != ChatUIMode::SearchMode) {
        return;
    }

    // 找到鼠标下面的那个控件, 然后一层层往上找 ——
    // 因为点在列表里时, 拿到的往往是"列表内部的子控件", 不是列表本身
    QWidget *w = QApplication::widgetAt(event->globalPos());
    while(w != nullptr && w!= this) {
        if (w == ui->search_list || w == ui->search_edit || w == ui->add_btn) {
            return;
        }
        w = w->parentWidget();
    }

    ShowSearch(false);
    // 顺手把"查找好友"的弹窗也关掉(它可能还开着)
    ui->search_list->CloseFindDlg();
}

bool ChatDialog::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress) {
        handleGlobalMousePress(static_cast<QMouseEvent*>(event));
    }

    return QDialog::eventFilter(watched, event);
}

// ------------------------------------------------------------
// 侧边栏
// ------------------------------------------------------------
void ChatDialog::slot_side_chat()
{
    ClearLabelState(ui->side_chat_lb);
    ui->stackedWidget->setCurrentWidget(ui->chat_page);
    _state = ChatUIMode::ChatMode;
    ShowSearch(false);
    qDebug() << "receive side chat clicked";
}

void ChatDialog::slot_side_contact()
{
    ClearLabelState(ui->side_contact_lb);
    _state = ChatUIMode::ContactMode;

    // 每次进联系人页都按"当前好友列表"重灌一遍:
    // 刚同意的好友申请、刚开机收到的离线消息, 都会立刻反映出来
    ui->con_user_list->LoadFriendList();

    ShowSearch(false);

    // 右栏: 如果正停在好友资料页就留着, 否则回到会话页
    if (ui->stackedWidget->currentWidget() != ui->friend_info_page) {
        ui->stackedWidget->setCurrentWidget(ui->chat_page);
    }

    qDebug() << "receive side contact clicked";
}

void ChatDialog::slot_side_apply()
{
    ClearLabelState(ui->side_apply_lb);
    ui->stackedWidget->setCurrentWidget(ui->friend_apply_page);   // 右半区切到申请页(现在是空页)
    ShowSearch(false);
    qDebug() << "receive side apply clicked";
}

// ------------------------------------------------------------
// "+" 按钮: 展开/收起搜索列表
// ------------------------------------------------------------
void ChatDialog::slot_add_btn_clicked()
{
    if (_mode == ChatUIMode::SearchMode) {
        // 已经在搜索 -> 收起, 回到 _state 记着的那一页
        ShowSearch(false);
        ui->chat_page->SetTitle(_state == ChatUIMode::ContactMode ? tr("联系人") : tr("聊天"));
    } else {
        _state = _mode;          //  记住"搜索之前在哪一页"
        ShowSearch(true);
        ui->chat_page->SetTitle(tr("搜索"));
        ui->search_edit->setFocus();   // 展开就直接能打字
    }
    //qDebug() << "[chatdlg] add_btn clicked, mode =" << static_cast<int>(_mode);
}

// ------------------------------------------------------------
// 列表滚到底
//  会在这里发分页请求; _b_loading 用来防止"上一次还没回来又发一次"
// ------------------------------------------------------------
void ChatDialog::slot_loading_chat_user()
{
    if (_b_loading) {
        return;
    }

    _b_loading = true;
    // LoadingDlg* loadingDialog = new LoadingDlg(this, tr("加载中..."));
    // loadingDialog->setModal(true);      // 模态: 加载期间不让点别的
    // loadingDialog->show();

    qDebug() << "[chatdlg] loading more chat user... ";
    // //   deleteLater 是"等回到事件循环再删", 安全。
    // loadingDialog->deleteLater();

    _b_loading = false;
}

void ChatDialog::slot_text_changed(const QString &str)
{
    if (!str.isEmpty()) {
        ShowSearch(true);
    }
}

void ChatDialog::slot_friend_auth(const QJsonObject &json)
{
    if(json["error"] != 0) {
        return;
    }
    const int uid = json["uid"].toInt();
    if (uid == 0) {
        return;
    }
    //  已经在列表里就只刷新, 不再插一行
    if (_user_items.contains(uid)) {
        qDebug() << "[chatdlg] 对方同意了我, uid 已在列表, 跳过插入 =" << uid;
        return;
    }
    QString show = json["nick"].toString();
    if(show.isEmpty()) {
        show = json["name"].toString();
    }
    QString icon = json["icon"].toString();
    if (icon.isEmpty()) {
        icon = ":/res/head_1.jpg";
    }

    addChatUserItem(show, icon, tr("暂无消息"), uid);
}

void ChatDialog::slot_chat_user_clicked(QListWidgetItem* item)
{
    if(item == nullptr) return;
    const int uid = item->data(Qt::UserRole).toInt();
    SetUnread(uid, false);
    const int thread_id = item->data(Qt::UserRole + 1).toInt();
    const QString name = item->data(Qt::UserRole + 2).toString();

    QString head;
    auto hit = _user_items.find(uid);
    if (hit != _user_items.end() && hit.value() != nullptr) {
        head = hit.value()->GetHead();
    }
    ui->chat_page->SetSession(thread_id, uid, name, head);

    ClearLabelState(ui->side_chat_lb);
    ui->side_chat_lb->SetSelected(true);
    ui->stackedWidget->setCurrentWidget(ui->chat_page);
    ShowSearch(false);

    qDebug() << "[chatdlg] open chat with uid =" << uid << " thread_id =" << thread_id;
}

// ------------------------------------------------------------
// 点联系人列表里的一行 -> 右侧切到"好友资料页"
//   注意: 这里【不】切到会话页, 也不打开聊天 —— 和微信一样,
//   资料页上点"发消息"才真的进会话。
// ------------------------------------------------------------
void ChatDialog::slot_friend_info_page(std::shared_ptr<FriendInfo> friend_info)
{
    if (!friend_info) {
        return;
    }

    ui->friend_info_page->SetInfo(friend_info);
    ui->stackedWidget->setCurrentWidget(ui->friend_info_page);

    qDebug() << "[chatdlg] show friend info page, uid =" << friend_info->_uid;
}

// ------------------------------------------------------------
// 资料页点"发消息" -> 切回会话页, 并替用户把会话行点开
//   复用 slot_chat_user_clicked(): 那里已经把"清红点 + SetSession + 切页"都做好了,
//   所以这里只要找到/造出那一行, 然后直接调它就行。
// ------------------------------------------------------------
void ChatDialog::slot_jump_chat_item_from_infopage(std::shared_ptr<FriendInfo> friend_info)
{
    if (!friend_info) {
        return;
    }
    const int uid = friend_info->_uid;
    if (uid <= 0) {
        return;
    }

    // 1) 左栏里找这个人的会话行
    QListWidgetItem *target = nullptr;
    for (int i = 0; i < ui->chat_user_list->count(); ++i) {
        QListWidgetItem *it = ui->chat_user_list->item(i);
        if (it != nullptr && it->data(Qt::UserRole).toInt() == uid) {
            target = it;
            break;
        }
    }

    // 2) 还没聊过 -> 现造一行(thread_id 用资料里带的, 可能是 0:
    //    发第一条消息时服务端会回真正的 thread_id, 再由 sig_thread_id_updated 回填)
    if (target == nullptr) {
        addChatUserItem(friend_info->GetShowName(), friend_info->_icon,
                        tr("暂无消息"), uid, friend_info->_thread_id);
        for (int i = 0; i < ui->chat_user_list->count(); ++i) {
            QListWidgetItem *it = ui->chat_user_list->item(i);
            if (it != nullptr && it->data(Qt::UserRole).toInt() == uid) {
                target = it;
                break;
            }
        }
    }

    if (target == nullptr) {
        qWarning() << "[chatdlg] jump to chat failed, uid =" << uid;
        return;
    }

    ui->chat_user_list->setCurrentItem(target);
    slot_chat_user_clicked(target);     // 等价于用户自己点了这一行
}

// ------------------------------------------------------------
// 1026: 真实联系人列表
// ------------------------------------------------------------
void ChatDialog::slot_contacts_loaded(const QJsonObject& json)
{
    const int err = json["error"].toInt();
    if (err != 0) {
        return;
    }

    // 1026 是"全量联系人"回包 -> 先清空再灌, 防止刷新时残留旧数据
    UserMgr::GetInstance()->ClearFriendList();

    // ★ 千万别自己 delete 这些行控件!
    //   它们是 setItemWidget() 贴进格子的(见 addChatUserItem), 所有权已经交给 QListWidget;
    //   自己 delete 会在列表内部留下野指针, 下一次重绘/清理时就是"释放一个已经释放的对象"
    //   -> 异常码 0xc0000005, 崩在 Qt5Core.dll(和 10/13 那次 ChatView::removeAllItem 一个道理)
    //   clear() 让列表自己把 item 和贴上去的控件一起收掉
    ui->chat_user_list->clear();
    _user_items.clear();

    const QJsonArray friends = json["friends"].toArray();
    for (const auto& v : friends) {
        const QJsonObject o = v.toObject();

        // 显示名优先级: 备注名(back) > 昵称(nick) > 用户名(name)
        QString show = o["back"].toString();
        if (show.isEmpty()) show = o["nick"].toString();
        if (show.isEmpty()) show = o["name"].toString();

        QString icon = o["icon"].toString();
        if (icon.isEmpty()) icon = ":/res/head_1.jpg";

        const int tid = o["thread_id"].toInt();
        addChatUserItem(show, icon, tr("暂无消息"), o["uid"].toInt(),tid);

        // ★ 同时存一份到 UserMgr: 联系人页(ContactUserList)和好友资料页
        //   (FriendInfoPage)都从这里取数据, 而不是各自再解析一遍 JSON
        UserMgr::GetInstance()->AddFriend(std::make_shared<FriendInfo>(
            o["uid"].toInt(),
            o["name"].toString(),
            o["nick"].toString(),
            o["desc"].toString(),
            o["sex"].toInt(),
            icon,
            o["back"].toString(),
            tid));
    }

    // 重建完把"未读红点 + 最后一条消息"贴回去 —— 1026 的报文里不带这两样
    const QSet<int> unread = _unread_uids;      // 拷一份再遍历(SetUnread 会动 _unread_uids)
    for (int uid : unread) {
        SetUnread(uid, true);
    }
    for (auto it = _last_msg_text.begin(); it != _last_msg_text.end(); ++it) {
        auto hit = _user_items.find(it.key());
        if (hit != _user_items.end() && hit.value() != nullptr) {
            // 时间也要贴回来: 这一行是刚 new 出来的, 时间标签本来空着
            hit.value()->SetLastMsg(it.value(), _last_msg_time.value(it.key()));
        }
    }

    ui->chat_page->SetTitle(tr("联系人 (%1)").arg(friends.size()));

    const QJsonArray applies = json["apply_list"].toArray();
    for(const auto& v : applies) {
        const QJsonObject o = v.toObject();
        auto info = std::make_shared<ApplyInfo>(
            o["uid"].toInt(),o["name"].toString(), o["desc"].toString(),
            o["icon"].toString(), o["nick"].toString(), o["sex"].toInt(),
            o["status"].toInt()
            );
        ui->friend_apply_page->AddNewApply(info);
    }
}

void ChatDialog::slot_friend_apply(std::shared_ptr<ApplyInfo> info)
{
    if (info == nullptr) {
        return;
    }

    ui->friend_apply_page->AddNewApply(info);

    // 顺手把界面切到"申请页", 否则用户根本不知道有人申请他
    ClearLabelState(ui->side_apply_lb);     //先把别的高亮清理掉
    ui->side_apply_lb->SetSelected(true);   //把"申请"按钮点亮
    ui->stackedWidget->setCurrentWidget(ui->friend_apply_page);
}

void ChatDialog::slot_recv_msg(const QJsonObject& json)
{
    const int from = json["from_uid"].toInt();
    if (from == 0) {
        return;
    }

    if (ui->chat_page->GetPeerId() != from) {
        SetUnread(from, true);
    }

    updateChatUserThreadId(from, json["thread_id"].toInt());
    _last_msg_text[from] = json["content"].toString();   // 记下来: 重建列表时要贴回左栏

    // 时间怎么算:
    //   实时转发来的消息, 服务端报文里【没有】create_time -> 就是"现在";
    //   登录后补投的离线消息带着它自己的发送时间 -> 显示真实时间(而不是你登录的时间)
    QString when = FormatChatTime(json["create_time"].toString());
    if (when.isEmpty()) {
        when = QTime::currentTime().toString("HH:mm");
    }
    _last_msg_time[from] = when;

    auto it = _user_items.find(from);
    if (it != _user_items.end() && it.value() != nullptr) {
        it.value()->SetLastMsg(json["content"].toString(), when);
    }else{
        QString icon = json["icon"].toString();
        if (icon.isEmpty()) {
            icon = ":/res/head_1.jpg";
        }
        QString nick = json["nick"].toString();
        if (nick.isEmpty()) {
            nick = json["name"].toString();
        }

        addChatUserItem(nick, icon, json["content"].toString(), from,json["thread_id"].toInt());
    }
}

void ChatDialog::slot_new_friend(const QJsonObject &json)
{
    const int uid = json["uid"].toInt();
    if (uid <= 0) {
        return;
    }
    if (_user_items.contains(uid)) {
        return;
    }

    QString name = json["nick"].toString();
    if (name.isEmpty()) {
        name = json["name"].toString();
    }
    QString icon = json["icon"].toString();
    if (icon.isEmpty()) {
        icon = ":/res/head_1.jpg";
    }

    // thread_id 先给 0: 还没有会话, 第一次发消息时服务端会建(1026 下次登录也会带上)
    addChatUserItem(name, icon, tr("暂无消息"), uid, 0);

    // 顺手把"联系人 (N)"的标题更新
    ui->chat_page->SetTitle(tr("联系人 (%1)").arg(_user_items.size()));
}

void ChatDialog::slot_msg_sent(int uid, const QString &text, const QString &server_time)
{
    _last_msg_text[uid] = text;      // 同上: 重建列表时要贴回左栏

    // 时间: 从历史里带出来的消息用服务端给的真实时间;
    //       自己刚发出去的那种(server_time 为空)才用本机当前时间。
    QString when = FormatChatTime(server_time);
    if (when.isEmpty()) {
        when = QTime::currentTime().toString("HH:mm");
    }
    _last_msg_time[uid] = when;      // 记住它, 1026 重建列表后要贴回去

    auto it = _user_items.find(uid);
    if(it!=_user_items.end() && it.value() != nullptr){
        it.value()->SetLastMsg(text, when);
    }
}

void ChatDialog::SetUnread(int uid, bool unread){
    // 记进成员: 1026 重建列表后要按它把红点贴回去(1026 报文里没有未读信息)
    if (unread) {
        _unread_uids.insert(uid);
    } else {
        _unread_uids.remove(uid);
    }

    auto it = _user_items.find(uid);
    if(it!=_user_items.end() && it.value() != nullptr) {
        it.value()->ShowRedPoint(unread);
    }
}