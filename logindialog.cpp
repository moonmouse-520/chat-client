#include "logindialog.h"
#include "ui_logindialog.h"
#include "tcpmgr.h"
LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    connect(ui->reg_btn, &QPushButton::clicked, this, &LoginDialog::swichRegister);
    ui->pass_edit->setEchoMode(QLineEdit::Password);

    ui->forget_label->SetState("normal", "hover","","selected","selected_hover","");
    connect(ui->forget_label,  &ClickedLabel::clicked, this, &LoginDialog::slot_forget_pwd);
    initHead();
    initHttpHandlers();
    //连接登录回包信号
    connect(HttpMgr::GetInstance().get(),&HttpMgr::sig_log_mod_finish, this, &LoginDialog::slot_log_mod_finish);

    connect(this, &LoginDialog::sig_connect_tcp, TcpMgr::GetInstance().get(),&TcpMgr::slot_tcp_connect);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_con_success, this, &LoginDialog::slot_tcp_con_finish);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_login_failed,this,&LoginDialog::slog_login_failed);
}

LoginDialog::~LoginDialog()
{
    qDebug() << "LoginDialog distruct";
    delete ui;
}

void LoginDialog::initHttpHandlers()
{
    _handlers.insert(ReqId::ID_LOGIN_USER, [this](QJsonObject jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            QString tip;
            switch (error) {
            case 1001: tip = tr("请求参数错误(客户端和服务端字段不一致)"); break;
            case 1006: tip = tr("密码错误"); break;                      // PasswdErr
            case 1009: tip = tr("密码错误(或客户端/数据库密码编码不一致)"); break; // PasswdInvalid
            case 1010: tip = tr("登录状态失效, 请重新登录"); break;        // TokenInvalid
            default:   tip = tr("登录失败, 错误码 %1").arg(error); break;
            }
            qDebug() << "Login failed, error is: " << error;
            showTip(tip, false);
            enableBtn(true);
            return;
        }
        auto email = jsonObj["email"].toString();
        ServerInfo si;
        si.Uid = jsonObj["uid"].toInt();
        _uid   = si.Uid;
        si.Host = jsonObj["host"].toString();
        si.Port = jsonObj["port"].toString();
        si.Token = jsonObj["token"].toString();
        _token = si.Token;

        qDebug() << "email is " << email << ", uid is " << si.Uid << ", host is "<<si.Host<<", Port is " << si.Port << ", Token is " << si.Token ;

        emit sig_connect_tcp(si);
    });
}

void LoginDialog::initHead()
{
    QPixmap originalPixmap(":/res/head_1.jpg");
    qDebug() << originalPixmap.size() << ui->head_label->size();
    originalPixmap = originalPixmap.scaled(ui->head_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    //创建一个和原始图片相同大学的QPixMap 用于绘制圆角图片
    QPixmap roundedPixmap(originalPixmap.size());
    roundedPixmap.fill(Qt::transparent);

    QPainter painter(&roundedPixmap);
    painter.setRenderHint(QPainter::Antialiasing);  //设置抗锯齿, 让圆角更平滑
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    //使用QPainterPath设置圆角
    QPainterPath path;
    path.addRoundedRect(0,0,originalPixmap.width(),originalPixmap.height(),10,10);
    painter.setClipPath(path);

    //将原始图片绘制到roundedPixmap上
    painter.drawPixmap(0,0,originalPixmap);
    //设置绘制好的圆角到QLabel上
    ui->head_label->setPixmap(roundedPixmap);
}

void LoginDialog::AddTipErr(TipErr te, QString tips)
{
    _tip_errs[te] = tips;
    showTip(tips, false);
}

void LoginDialog::DelTipErr(TipErr te)
{
    _tip_errs.remove(te);
    if(_tip_errs.empty()){
        ui->err_tip->clear();
        return;
    }
    showTip(_tip_errs.first(), false);
}

bool LoginDialog::checkUserValid()
{
    auto email = ui->email_edit->text();
    if(email.isEmpty()) {
        qDebug() << "email is empty!" ;
        AddTipErr(TipErr::TIP_EMAIL_ERR, tr("邮箱不能为空"));
        return false;
    }
    DelTipErr(TipErr::TIP_EMAIL_ERR);
    return true;
}

bool LoginDialog::checkPwdValid()
{
    auto pass = ui->pass_edit->text();

    if(pass.length() < 6 || pass.length() > 15){
        AddTipErr(TipErr::TIP_PWD_ERR, tr("密码长度应为6~15"));
        return false;
    }

    QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*.]{6,15}$");
    bool match = regExp.match(pass).hasMatch();
    if(!match){
        AddTipErr(TipErr::TIP_PWD_ERR, tr("不能包含非法字符"));
        return false;
    }
    DelTipErr(TipErr::TIP_PWD_ERR);

    return true;

}

bool LoginDialog::enableBtn(bool enabled)
{
    ui->login_btn->setEnabled(enabled);
    ui->reg_btn->setEnabled(enabled);
    return true;
}

void LoginDialog::showTip(QString str, bool b_ok)
{
    if(b_ok) {
        ui->err_tip->setProperty("state", "normal");
    }else{
        ui->err_tip->setProperty("state", "err");
    }

    ui->err_tip->setText(str);
    repolish(ui->err_tip);
}
void LoginDialog::slot_forget_pwd()
{
    emit switchReset();
}

void LoginDialog::slog_login_failed(int err)
{
    QString result = QString("登录失败, err is %1").arg(err);
    showTip(result,false);
    enableBtn(true);
}

void LoginDialog::slot_tcp_con_finish(bool bsuccess)
{
    if(bsuccess) {
        showTip(tr("聊天服务连接成功, ====正在登录===="),true);
        QJsonObject jsonObj;
        jsonObj["uid"] = _uid;
        jsonObj["token"] = _token;
        QJsonDocument doc(jsonObj);
        QString jsonString = doc.toJson(QJsonDocument::Indented);

        //发送tcp请求给chat server
        TcpMgr::GetInstance()->sig_send_data(ReqId::ID_CHAT_LOGIN, jsonString);
    }else{
        showTip(tr("网络异常"),false);
        enableBtn(true);
    }
}

void LoginDialog::on_login_btn_clicked()
{
    if(checkUserValid() == false) {
        return;
    }

    if(checkPwdValid() == false) {
        return;
    }

    enableBtn(false);
    auto email = ui->email_edit->text();
    auto pwd = ui->pass_edit->text();
    //发送http请求
    QJsonObject json_obj;
    json_obj["email"] = email;
    json_obj["passwd"] = xorString(pwd);
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/user_login"), json_obj, ReqId::ID_LOGIN_USER, Modules::LOGIBMOD);
}

void LoginDialog::slot_log_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if(err != ErrorCodes::SUCCESS){
        showTip("网络请求错误", false);
        enableBtn(true);
        return;
    }

    //解析Json
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    //Json解析错误
    if(jsonDoc.isNull()){
        showTip(tr("服务端返回异常, 请稍后重试"), false);
        enableBtn(true);
        return;
    }

    if(!jsonDoc.isObject()){
        showTip(tr("服务端返回异常, 请稍后重试"), false);
        enableBtn(true);
        return;
    }
    auto it = _handlers.find(id);
    if(it == _handlers.end()) {
        showTip(tr("未知的响应类型 %1").arg(static_cast<int>(id)), false);
        enableBtn(true);                   // LoginDialog 里要加这行; 另两个对话框没有按钮门控
        return;
    }

    //按 ReqId 分发到对应的处理函数
    _handlers[id](jsonDoc.object());
    return;
}

