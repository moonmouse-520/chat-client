#ifndef TCPMGR_H
#define TCPMGR_H
#include <QTcpSocket>
#include "global.h"
#include "singleton.h"
#include <functional>
#include <QObject>
#include <memory>
#include <QMap>
#include <QJsonObject>
#include "userdata.h"
class TcpMgr : public QObject, public Singleton<TcpMgr>, public std::enable_shared_from_this<TcpMgr>
{
    Q_OBJECT
    friend class Singleton<TcpMgr>;
public:
    ~TcpMgr() = default;
private:
    TcpMgr();
    void initHandlers();
    void MsgHandler(ReqId id, int len, QByteArray data);
    void resetParseState();
    QTcpSocket _socket;
    QString _host;
    uint16_t _port;
    QByteArray _buffer;
    bool _b_recv_pending;
    quint16 _message_id;
    quint16 _message_len;
    QMap<ReqId, std::function<void(ReqId id, int len, QByteArray data)>> _handlers;
public slots:
    void slot_tcp_connect(ServerInfo);
    void slot_send_data(ReqId reqId, QString data);
signals:
    void sig_con_success(bool bsuccess);
    void sig_send_data(ReqId reqId, QString data);
    void sig_switch_chatdlg();
    void sig_login_failed(int);
    void sig_contacts_loaded(const QJsonObject& json); // 收到 1026(好友列表 + 待处理申请)后, 把整个 JSON 透传给界面
    void sig_user_search(std::shared_ptr<SearchInfo> si);
    void sig_add_friend_rsp(int err);
    void sig_friend_apply(std::shared_ptr<ApplyInfo> info);
    void sig_auth_rsp(const QJsonObject& json);
    void sig_friend_auth(const QJsonObject& json);
    void sig_send_msg_rsp(const QJsonObject& json);
    void sig_recv_msg(const QJsonObject& json);
    void sig_load_msg_rsp(const QJsonObject& json);
};

#endif // TCPMGR_H
