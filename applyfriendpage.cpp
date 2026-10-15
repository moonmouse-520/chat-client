#include "applyfriendpage.h"
#include "ui_applyfriendpage.h"
#include <QPainter>
#include <QMessageBox>
#include "authenfriend.h"
#include "tcpmgr.h"
ApplyFriendPage::ApplyFriendPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ApplyFriendPage)
{
    ui->setupUi(this);

    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_auth_rsp,this,&ApplyFriendPage::slot_auth_rsp);
}

ApplyFriendPage::~ApplyFriendPage()
{
    delete ui;
}

void ApplyFriendPage::AddNewApply(std::shared_ptr<ApplyInfo> info)
{
    if(info == nullptr) {
        return;
    }

    if (_apply_items.find(info->_uid) != _apply_items.end()) {
        return;
    }

    auto* item = new ApplyFriendItem();
    item->SetInfo(info);
    // 这一行被点了"添加"
    connect(item, &ApplyFriendItem::sig_auth_friend, this, [this](std::shared_ptr<ApplyInfo> info) {    _last_auth_uid = info->_uid;     // 记下正在认证谁, 1014 失败时要用

        auto *dlg = new AuthenFriend(this);
        dlg->SetApplyInfo(info);
        dlg->show();
    });
    // 老套路: 格子 + setSizeHint + setItemWidget
    auto *lw = new QListWidgetItem;
        lw->setSizeHint(item->sizeHint());
    ui->apply_friend_list->insertItem(0, lw);   // 最新在最上面
    ui->apply_friend_list->setItemWidget(lw, item);

    _apply_items[info->_uid] = item;
}

void ApplyFriendPage::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}


void ApplyFriendPage::slot_auth_rsp(const QJsonObject &json)
{
    const int err = json["error"].toInt();
        if (err != 0) {
        QString tip;
        switch (err) {
        case 1015: tip = tr("你们已经是好友了"); break;          // AlreadyFriend
        case 1016: tip = tr("该申请已被处理或已过期"); break;      // ApplyExpired
        case 1011: tip = tr("对方账号无效"); break;               // UidInvalid
        case 1010: tip = tr("登录状态失效, 请重新登录"); break;     // TokenInvalid
        default:   tip = tr("操作失败, 错误码 %1").arg(err); break;
        }
        QMessageBox::information(this, tr("提示"), tip);          // 失败也必须有反馈

        // 申请已经不再成立了 -> 把那一行置成已添加, 免得用户反复点
        if (err == 1015) {
            const int uid = json["uid"].toInt();
            auto it = _apply_items.find(_last_auth_uid);
            if (it != _apply_items.end()) {
                it->second->showAddBtn(false);
            }
            emit sig_new_friend(json);
            qDebug() << "[applypage] 已是好友 uid =" << uid;
        }else if(err == 1016){
            auto it = _apply_items.find(_last_auth_uid);
            if(it != _apply_items.end()) {
                it->second->showAddBtn(false);
            }
        }
        return;
    }

    const int uid = json["uid"].toInt();
    auto it = _apply_items.find(uid);
    if(it != _apply_items.end()) {
        it->second->showAddBtn(false);
    }
    qDebug() << "[applypage] 已同意 uid =" << uid;
}
