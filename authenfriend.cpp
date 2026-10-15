#include "authenfriend.h"
#include "ui_authenfriend.h"
#include <QJsonObject>
#include <QJsonDocument>
#include "tcpmgr.h"
#include "global.h"
AuthenFriend::AuthenFriend(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AuthenFriend)
{
    ui->setupUi(this);
    setWindowTitle(tr("好友认证"));
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    ui->agree_btn->SetState("normal", "hover", "press");
    ui->reject_btn->SetState("normal", "hover", "press");
    setModal(true);
}

AuthenFriend::~AuthenFriend()
{
    delete ui;
}

void AuthenFriend::SetApplyInfo(std::shared_ptr<ApplyInfo> info)
{
    _info = info;
    ui->tip_lb->setText(tr("%1 请求添加你为好友").arg(info->_name));
    ui->back_ed->setPlaceholderText(info->_name + tr(" (备注名, 可留空)"));
}

void AuthenFriend::on_agree_btn_clicked()
{
    if(_info == nullptr) {
        return;
    }

    QJsonObject req;
    req["applyuid"] = _info->_uid;  // 申请人(对方)的 uid
    // 注意: 不用发 fromuid —— 服务端从 session 里拿"我是谁"
    // 给"他"起的备注名。空串也照发: 服务端看到空会用对方昵称兜底, 不会存成空白
    req["back"] = ui->back_ed->text().trimmed();

    qDebug() << "[authen] 我要发 1013:" << QString(QJsonDocument(req).toJson(QJsonDocument::Compact));
    emit TcpMgr::GetInstance()->sig_send_data(
        ReqId::ID_AUTH_FRIEND_REQ,
        QString(QJsonDocument(req).toJson(QJsonDocument::Compact)));
    this->hide();
    this->deleteLater();
}

void AuthenFriend::on_reject_btn_clicked()
{
    this->hide();
    this->deleteLater();
}

