#ifndef CHATPAGE_H
#define CHATPAGE_H

#include <QWidget>

namespace Ui {
class ChatPage;
}

class ChatPage : public QWidget
{
    Q_OBJECT

public:
    explicit ChatPage(QWidget *parent = nullptr);
    ~ChatPage();
    void SetTitle(const QString& title);
    void SetSession(int thread_id, int peer_uid, const QString &name, const QString &icon = QString());
    int GetPeerId() const {return _peer_id;}
private:
    Ui::ChatPage *ui;
    QString _last_sent_text;   // 发送前记下文本(1018 里可能不带 content)
    int _thread_id = 0;
    int _peer_id = 0;
    QString _peer_icon;        // 当前会话对方的头像路径
    qint64 _oldest_msg_id = 0;
    bool   _loading_history = false;
    bool   _b_prepend = false;
protected:
    void paintEvent(QPaintEvent *event) override;
private slots:
    void on_send_btn_clicked();
    void slot_send_msg_rsp(const QJsonObject& json);
    void slot_recv_msg(const QJsonObject& json);
    void slot_load_msg_rsp(const QJsonObject& json);
    void slot_load_more();

signals:
    // 第三个参数 = 这条消息在服务端的原始时间(空 = 就是"现在", 交给 ChatDialog 兜底)
    void sig_msg_sent(int peer_uid, const QString& text, const QString& server_time = QString());
    void sig_thread_id_updated(int peer_uid, int thread_id);
};

#endif // CHATPAGE_H
