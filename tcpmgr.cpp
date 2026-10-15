#include "tcpmgr.h"
#include <QAbstractSocket>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
#include "usermgr.h"
TcpMgr::TcpMgr() : _host(""),_port(0),_b_recv_pending(false),_message_id(0),_message_len(0)
{
    QObject::connect(&_socket, &QTcpSocket::connected, this, [this](){
        qDebug() << "Connected to Server";
        emit sig_con_success(true);
    });

    QObject::connect(&_socket, &QTcpSocket::readyRead, this, [this](){
        _buffer.append(_socket.readAll());

        forever{
            if(!_b_recv_pending){
                if(_buffer.size() < static_cast<int>(sizeof(quint16) * 2)){
                    return;//数据不够
                }
                // ★ 流必须"每轮读头部时新建"：QDataStream 的读位置只随"读头部"前进 4 字节，
                //   不会跳过 body。一旦把流建到循环外，第 2 条消息的头部就会从错误偏移读起。
                QDataStream stream(_buffer);
                stream.setVersion(QDataStream::Qt_5_12);
                stream >> _message_id >> _message_len;
                _buffer.remove(0,sizeof(quint16) * 2);
                qDebug() <<  "Message ID is: " << _message_id <<", Length is: " << _message_len;
            }


            if (_buffer.size() < _message_len) {
                _b_recv_pending = true;
                return;
            }
            _b_recv_pending = false;
            QByteArray messageBody = _buffer.mid(0,_message_len);
            qDebug() << "Receive msgbody is: " << messageBody;
            _buffer = _buffer.mid(_message_len);
            MsgHandler(ReqId(_message_id), _message_len, messageBody);
        }
    });


    QObject::connect(&_socket, &QAbstractSocket::errorOccurred, this,
            [this](QAbstractSocket::SocketError socketError){
                 switch (socketError) {
                 case QAbstractSocket::ConnectionRefusedError:
                     qDebug() << "Connection refused" ;
                     emit sig_con_success(false);
                     break;
                 case QAbstractSocket::RemoteHostClosedError:
                     qDebug() << "Remote Host Closed Connection";
                     emit sig_con_success(false);
                     break;
                 case QAbstractSocket::HostNotFoundError:
                     qDebug() << "Host Not Found";
                     emit sig_con_success(false);
                     break;
                 case QAbstractSocket::SocketTimeoutError:
                     qDebug() << "Connection Timeout";
                     emit sig_con_success(false);
                     break;
                 case QAbstractSocket::NetworkError:
                     qDebug() << "Network Error";
                     emit sig_con_success(false);
                     break;
                 default:
                     qDebug() << "Other Error";
                     emit sig_con_success(false);
                     break;
                 }
             });

    QObject::connect(&_socket, &QTcpSocket::disconnected, this, [this](){
        qDebug() << "Disconnected from server" ;
        resetParseState();   // 断线即清：否则半条消息会赖在 _buffer 里
    });

    //连接发送信号发送数据
    QObject::connect(this, &TcpMgr::sig_send_data,this, &TcpMgr::slot_send_data);
    //注册消息
    initHandlers();
}

void TcpMgr::initHandlers()
{
    _handlers.insert(ID_CHAT_LOGIN_RSP,[this](ReqId id, int len, QByteArray data){
        Q_UNUSED(len);
        qDebug() << "handle id is: " << id << ", data is: " << data;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        if(jsonDoc.isNull()){
            qDebug() << "Failed to create QjsonDoc";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        if(!jsonObj.contains("error")){
            qDebug() << "Login failed, Json Parse Err " << ErrorCodes::ERR_JSON;
            emit sig_login_failed(ErrorCodes::ERR_JSON);
            return;
        }

        int err = jsonDoc["error"].toInt();
        if(err!=ErrorCodes::SUCCESS){
            qDebug() << "Login failed, error is: "<< err;
            emit sig_login_failed(err);
            return;
        }

        auto userMgr = UserMgr::GetInstance();
        userMgr->SetUid(jsonObj["uid"].toInt());
        userMgr->SetName(jsonObj["name"].toString());
        userMgr->SetToken(jsonObj["token"].toString());
        emit sig_switch_chatdlg();

        // 拉好友列表 + 待处理申请
        QJsonObject req;
        req["uid"] = userMgr->GetUid();
        emit sig_send_data(ReqId::ID_LOAD_CHAT_THREAD_REQ,QString(QJsonDocument(req).toJson(QJsonDocument::Compact)));
    });

    _handlers.insert(ID_LOAD_CHAT_THREAD_RSP, [this](ReqId id, int len, QByteArray data){
        Q_UNUSED(id);
        Q_UNUSED(len);
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if(doc.isNull()) {
            qDebug() << "[tcp] 1026 json parse failed";
            return;
        }

        emit sig_contacts_loaded(doc.object());
    });

    _handlers.insert(ID_SEARCH_USER_RSP, [this](ReqId id, int len, QByteArray data){
        Q_UNUSED(id);
        Q_UNUSED(len);

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if(doc.isNull()) {
            emit sig_user_search(nullptr);
            return;
        }

        const QJsonObject o = doc.object();
        if(o["error"] != 0) {
            qDebug() << "[tcp] search failed, error =" << o["error"].toInt();
            emit sig_user_search(nullptr);
            return;
        }

        auto si = std::make_shared<SearchInfo>(
            o["uid"].toInt(),
            o["name"].toString(),
            o["nick"].toString(),
            o["desc"].toString(),
            o["sex"].toInt(),
            o["icon"].toString());
        qDebug() << "[tcp] search ok, uid =" << si->_uid << " name =" << si->_name;
        emit sig_user_search(si);
    });

    _handlers.insert(ID_ADD_FRIEND_RSP, [this](ReqId id, int len, QByteArray data){
        Q_UNUSED(id);
        Q_UNUSED(len);
        const QJsonDocument doc =  QJsonDocument::fromJson(data);
        const int err = doc.isNull() ? ErrorCodes::ERR_JSON : doc.object()["error"].toInt();
        qDebug() << "[tcp] add friend rsp, error =" << err;
        emit sig_add_friend_rsp(err);
    });

    _handlers.insert(ID_NOTIFY_ADD_FRIEND_REQ,[this](ReqId id, int len, QByteArray data){
        Q_UNUSED(id);
        Q_UNUSED(len);
        const QJsonDocument doc = QJsonDocument::fromJson(data);
        if(doc.isNull()) {
            return;
        }

        const QJsonObject o = doc.object();

        if (!o.contains("applyuid")) {
            return;
        }

        auto info = std::make_shared<ApplyInfo>(
            o["applyuid"].toInt(),      // uid
            o["name"].toString(),
            o["desc"].toString(),
            o["icon"].toString(),
            o["nick"].toString(),
            o["sex"].toInt(),
            0);
        qDebug() << "[tcp] new friend apply from uid =" << info->_uid;
        emit sig_friend_apply(info);
    });

    _handlers.insert(ID_AUTH_FRIEND_RSP, [this](ReqId id, int len, QByteArray data){
        Q_UNUSED(id); Q_UNUSED(len);
        const QJsonDocument doc = QJsonDocument::fromJson(data);
        if(doc.isNull()) {
            return;
        }
        qDebug() << "[tcp] auth rsp =" << doc.object()["error"].toInt();
        emit sig_auth_rsp(doc.object());
    });

    _handlers.insert(ID_NOTIFY_AUTH_FRIEND_REQ,[this](ReqId id, int len, QByteArray data){
        Q_UNUSED(id); Q_UNUSED(len);
        const QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull()) return;
        qDebug() << "[tcp] friend auth ok, uid =" << doc.object()["uid"].toInt();
        emit sig_friend_auth(doc.object());
    });

    _handlers.insert(ID_TEXT_CHAT_MSG_RSP,[this](ReqId id, int len, QByteArray data){
        Q_UNUSED(id);
        Q_UNUSED(len);
        const QJsonDocument doc = QJsonDocument::fromJson(data);
        if(doc.isNull()) return;
        qDebug() << "[tcp] send msg rsp =" << data;
        emit sig_send_msg_rsp(doc.object());
    });

    _handlers.insert(ID_NOTIFY_TEXT_CHAT_MSG_REQ,[this](ReqId id, int len, QByteArray data){
        Q_UNUSED(id);
        Q_UNUSED(len);
        const QJsonDocument doc = QJsonDocument::fromJson(data);
        if(doc.isNull()) return;
        qDebug() << "[tcp] recv msg =" << data;
        emit sig_recv_msg(doc.object());
    });

    _handlers.insert(ID_LOAD_CHAT_MSG_RSP,[this](ReqId id, int len, QByteArray data){
        Q_UNUSED(id); Q_UNUSED(len);
        const QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull()) {
            return;
        }
        qDebug() << "[tcp] load msg rsp =" << data;
        emit sig_load_msg_rsp(doc.object());
    });

}

void TcpMgr::MsgHandler(ReqId id, int len, QByteArray data)
{
    auto find_iter = _handlers.find(id);
    if(find_iter == _handlers.end()){
        qDebug() << "not found id: " << id << "to handle";
        return;
    }
    (*find_iter)(id,len,data);
}


void TcpMgr::resetParseState()
{
    _buffer.clear();
    _b_recv_pending = false;
    _message_id = 0;
    _message_len = 0;
}

void TcpMgr::slot_tcp_connect(ServerInfo si)
{
    qDebug() << "Receive tcp connect signal";
    qDebug() << "Connecting to server";

    _host = si.Host;
    _port = static_cast<uint16_t>(si.Port.toInt());
    _socket.connectToHost(_host,_port);
}

void TcpMgr::slot_send_data(ReqId reqId, QString data)
{
    uint16_t id = reqId;
    //将字符串转化为utf8编码的字节数组
    QByteArray dataBytes = data.toUtf8();

    //  协议头里的 len 是"字节数"，必须取 UTF-8 字节数组的长度：
    //   data.size() 是 UTF-16 码元数，中文会少算（"你好" → 2，而 UTF-8 是 6 字节）
    if (dataBytes.size() > 65535) {
        qDebug() << "send failed: body too large," << dataBytes.size();
        return;   // 截断会破坏帧边界，宁可不发
    }
    quint16 len = static_cast<quint16>(dataBytes.size());

    //创建一个字节数组储存要发的数据
    QByteArray block;
    QDataStream out(&block,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);   // 与接收侧保持一致

    //设置数据流使用网络字节序
    out.setByteOrder(QDataStream::BigEndian);

    out << id << len;
    block.append(dataBytes);

    _socket.write(block);
}
