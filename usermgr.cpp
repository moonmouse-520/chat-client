#include "usermgr.h"

UserMgr::~UserMgr()
{

}

// ------------------------------------------------------------
// 加好友: uid 不合法直接丢, 避免脏数据混进联系人列表
// ------------------------------------------------------------
void UserMgr::AddFriend(std::shared_ptr<FriendInfo> friend_info)
{
    if (friend_info == nullptr || friend_info->_uid <= 0) {
        return;
    }

    // QMap 的 operator[] : key 不存在就插入, 存在就覆盖
    // -> 同一个好友重复灌进来也只会有一条
    _friend_map[friend_info->_uid] = friend_info;
}

// ------------------------------------------------------------
// 按 uid 查好友
//   注意: find 返回的是"迭代器", 不能直接拿去用,
//   必须判断 != end() 再取 value, 否则就是访问越界
// ------------------------------------------------------------
std::shared_ptr<FriendInfo> UserMgr::GetFriendById(int uid)
{
    auto it = _friend_map.find(uid);
    if (it == _friend_map.end()) {
        return nullptr;      // 不是好友, 或者 1026 还没回来
    }
    return it.value();
}

// ------------------------------------------------------------
// 全部好友(联系人页遍历用)
//   返回 vector 的拷贝: 调用方遍历的过程中就算列表被清空也不怕
// ------------------------------------------------------------
std::vector<std::shared_ptr<FriendInfo>> UserMgr::GetFriendList()
{
    std::vector<std::shared_ptr<FriendInfo>> res;
    res.reserve(static_cast<size_t>(_friend_map.size()));

    for (auto it = _friend_map.begin(); it != _friend_map.end(); ++it) {
        res.push_back(it.value());
    }
    return res;
}

void UserMgr::ClearFriendList()
{
    _friend_map.clear();
}

UserMgr::UserMgr()
{

}
