#include "httpmgr.h"

HttpMgr::~HttpMgr()
{

}

HttpMgr::HttpMgr()
{
    _manager.setTransferTimeout(5000);
    connect(this, &HttpMgr::sig_http_finish, this, &HttpMgr::slot_http_finish);
}

void HttpMgr::PostHttpReq(QUrl url, QJsonObject json, ReqId req_id, Modules mod)
{
    QByteArray data = QJsonDocument(json).toJson();
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(data.length()));
    auto self = shared_from_this();
    QNetworkReply * reply = _manager.post(request, data);
    QObject::connect(reply, &QNetworkReply::finished, [self, reply, req_id, mod](){
        //处理错误情况
        QString res = QString::fromUtf8(reply->readAll());

        if(reply->error() != QNetworkReply::NoError) {
            qDebug() << "http error:" << reply->errorString() << "body:" << res;
            //发送信号通知完成
            emit self->sig_http_finish(req_id, res, ErrorCodes::ERR_NETWORK, mod);
            reply->deleteLater();
            return;
        }


        //发送信号通知完成
        emit self->sig_http_finish(req_id,res,ErrorCodes::SUCCESS,mod);
        reply->deleteLater();
    });

}

void HttpMgr::slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod)
{
    if(mod == Modules::REGISTERMOD) {
        //发送信号通知指定模块http的响应结束了
        emit sig_reg_mod_finish(id, res, err);
    }

    if(mod == Modules::RESETMOD) {
        //发送信号通知指定模块的http响应结束了
        emit sig_reset_mod_finish(id,res,err);
    }

    if(mod == Modules::LOGIBMOD){
        emit sig_log_mod_finish(id,res,err);
    }
}


