#ifndef USERMGR_H
#define USERMGR_H

#include <QObject>
#include <QString>
#include <QMap>
#include <memory>
#include <vector>
#include <singleton.h>
#include "userdata.h"

class UserMgr : public QObject, public Singleton<UserMgr>
{
    Q_OBJECT
    friend class Singleton<UserMgr>;
public:
    ~UserMgr();

    void SetUid(int uid){_uid = uid;};
    int GetUid(){return _uid;};
    void SetName(QString name) {_name = name;};
    QString GetName(){return _name;};
    void SetToken(QString token){_token = token;};
    QString GetToken(){return _token;};
    void SetIcon(QString icon){_icon = icon;};
    QString GetIcon(){return _icon;};


    // 收 1026 回包时灌进来(见 ChatDialog::slot_contacts_loaded)
    void AddFriend(std::shared_ptr<FriendInfo> friend_info);

    // 按 uid 查好友: 好友资料页靠它取数据; 不是好友返回 nullptr
    std::shared_ptr<FriendInfo> GetFriendById(int uid);

    // 联系人页要"全部好友" -> 返回一份拷贝出来的 vector
    std::vector<std::shared_ptr<FriendInfo>> GetFriendList();

    // 1026 是全量列表, 重新登录/刷新前先清空
    void ClearFriendList();

private:
    UserMgr();
    int _uid = 0;
    QString _name;
    QString _token;
    QString _icon;

    // ★ 用 QMap<int, ...> 而不是 vector:
    //   1) 资料页要按 uid 直接查 -> map 直接命中, 不用遍历;
    //   2) 同一个 uid 重复 AddFriend 天然覆盖, 不会出现两行联系人;
    QMap<int, std::shared_ptr<FriendInfo>> _friend_map;
signals:
};

#endif // USERMGR_H
