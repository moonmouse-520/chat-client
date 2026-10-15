#ifndef CHATUSERLIST_H
#define CHATUSERLIST_H

#include <QListWidget>
#include <QWheelEvent>
#include <QEvent>
#include <QScrollBar>
#include <QDebug>

class ChatUserList : public QListWidget
{
    Q_OBJECT
public:
    ChatUserList(QWidget *parent = nullptr);

protected:
    // 事件过滤器: 挂在 viewport 上, 用来处理"鼠标进出"和"滚轮"
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    // 滚到底了 -> 通知外面去加载下一页(教程第 20 集接这个信号)
    void sig_loading_chat_user();
};

#endif // CHATUSERLIST_H
