#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "httpmgr.h"
#include <QMap>
#include "global.h"
#include <QJsonObject>
#include <functional>
#include <QDebug>
#include <QPainter>
#include <QPixmap>
#include <QPainterPath>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private:
    void initHttpHandlers();
    void initHead();
    Ui::LoginDialog *ui;
    void AddTipErr(TipErr te, QString tips);
    void DelTipErr(TipErr te);
    bool checkUserValid();
    bool checkPwdValid();
    bool enableBtn(bool enabled);
    void showTip(QString,bool);

    QMap<TipErr, QString> _tip_errs;
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;
    int _uid;
    QString _token;
signals:
    void swichRegister();
    void switchReset();
    void sig_connect_tcp(ServerInfo);

public slots:
    void slot_forget_pwd();
    void slog_login_failed(int err);
    void slot_tcp_con_finish(bool bsuccess);
private slots:
    void on_login_btn_clicked();
    void slot_log_mod_finish(ReqId id, QString res, ErrorCodes err);
};


#endif // LOGINDIALOG_H
