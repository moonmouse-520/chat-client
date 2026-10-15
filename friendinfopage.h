#ifndef FRIENDINFOPAGE_H
#define FRIENDINFOPAGE_H

#include <QWidget>
#include <memory>
#include "userdata.h"

namespace Ui {
class FriendInfoPage;
}

// ============================================================
// FriendInfoPage —— 好友资料页(右侧大页面)
//
// 点联系人列表的一行 -> ChatDialog 切到这一页 -> SetInfo() 填数据
// 页面上"发消息"按钮 -> sig_jump_chat_item -> ChatDialog 切回会话页并打开该好友会话
// ============================================================
class FriendInfoPage : public QWidget
{
    Q_OBJECT
public:
    explicit FriendInfoPage(QWidget *parent = nullptr);
    ~FriendInfoPage();

    void SetInfo(std::shared_ptr<FriendInfo> friend_info);

private slots:
    void on_msg_chat_clicked();

signals:
    void sig_jump_chat_item(std::shared_ptr<FriendInfo> friend_info);

private:
    Ui::FriendInfoPage *ui;
    std::shared_ptr<FriendInfo> _friend_info;
};

#endif // FRIENDINFOPAGE_H
