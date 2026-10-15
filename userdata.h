#ifndef USERDATA_H
#define USERDATA_H

#include <QString>

class SearchInfo {
public:
    SearchInfo(int uid, QString name, QString nick, QString desc, int sex, QString icon)
        : _uid(uid), _name(name), _nick(nick), _desc(desc), _sex(sex), _icon(icon){}

    int _uid;
    QString _name;
    QString _nick;
    QString _desc;
    int     _sex;
    QString _icon;  // 头像资源路径
};

class ApplyInfo
{
public:
    ApplyInfo(int uid, QString name, QString desc, QString icon,
              QString nick, int sex, int status = 0)
        : _uid(uid), _name(name), _desc(desc), _icon(icon),
        _nick(nick), _sex(sex), _status(status) {}

    int     _uid;
    QString _name;
    QString _desc;
    QString _icon;
    QString _nick;
    int     _sex;
    int     _status;      // 0 = 未处理, 1 = 已同意
};

// ============================================================
// FriendInfo —— 好友信息(联系人页 / 好友资料页的数据)
//
// 字段和服务端 1026 回包的 friends 数组一一对应
// (ChatServer\LogicSystem.cpp:448-459 里就是这么填的):
//   uid / name / nick / desc / sex / icon / back / thread_id
// ============================================================
class FriendInfo
{
public:
    FriendInfo(int uid, QString name, QString nick, QString desc,
               int sex, QString icon, QString back, int thread_id)
        : _uid(uid), _name(name), _nick(nick), _desc(desc),
          _sex(sex), _icon(icon), _back(back), _thread_id(thread_id) {}

    int     _uid;
    QString _name;       // 用户名(注册时那个)
    QString _nick;       // 昵称
    QString _desc;       // 个性签名
    int     _sex;        // 0 = 男, 1 = 女
    QString _icon;       // 头像资源路径
    QString _back;       // ★ 备注名: 我给对方起的名字(申请时填的), 列表优先显示它
    int     _thread_id;  // 和这个人的会话 id(0 = 还没聊过)

    // 显示名优先级: 备注名 > 昵称 > 用户名
    //   写成成员函数而不是每处各写一遍, 免得以后三处不一致
    QString GetShowName() const {
        if (!_back.isEmpty()) return _back;
        if (!_nick.isEmpty()) return _nick;
        return _name;
    }
};
#endif // USERDATA_H
