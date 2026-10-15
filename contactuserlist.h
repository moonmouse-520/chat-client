#ifndef CONTACTUSERLIST_H
#define CONTACTUSERLIST_H

#include <QListWidget>
#include <memory>
#include "userdata.h"

class ContactUserList : public QListWidget
{
    Q_OBJECT
public:
    explicit ContactUserList(QWidget *parent = nullptr);

    // 按 UserMgr 当前的好友列表重新灌一遍(切到联系人页时调)
    void LoadFriendList();

private slots:
    void slot_item_clicked(QListWidgetItem *item);

signals:
    void sig_switch_friend_info_page(std::shared_ptr<FriendInfo> friend_info);
};

#endif // CONTACTUSERLIST_H
