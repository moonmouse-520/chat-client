#ifndef GLOBAL_H
#define GLOBAL_H
#include <memory>
#include <iostream>
#include <mutex>
#include <QWidget>
#include <functional>
#include "QStyle"
#include <QRegularExpression>
#include <QByteArray>
#include <QNetworkReply>
#include <QJsonObject>
#include <QUrl>
#include <QDir>
#include <QSettings>
/**
 * @brief repolish 用来刷新qss
 */
extern std::function<void(QWidget*)> repolish;

extern std::function<QString(QString)> xorString;

const int MIN_APPLY_LABEL_ED_LEN = 40;
const QString add_prefix = "添加标签 ";
// 标签之间 / 标签与容器边框的额外留白(像素), 算换行时都要加上
const int tip_offset = 5;
enum ReqId{
    ID_GET_VARIFY_CODE = 1001, //获取验证码
    ID_REG_USER = 1002, //注册用户
    ID_RESET_PWD = 1003,//重置密码
    ID_LOGIN_USER = 1004,//用户登录
    ID_CHAT_LOGIN = 1005,//登陆聊天服务器
    ID_CHAT_LOGIN_RSP = 1006,//登录聊天服务器回包
    ID_SEARCH_USER_REQ        = 1007, //搜索用户
    ID_SEARCH_USER_RSP        = 1008,
    ID_ADD_FRIEND_REQ         = 1009, //申请加好友
    ID_ADD_FRIEND_RSP         = 1010,
    ID_NOTIFY_ADD_FRIEND_REQ  = 1011, //别人申请加我(服务端主动推)
    ID_AUTH_FRIEND_REQ        = 1013, //同意加好友
    ID_AUTH_FRIEND_RSP        = 1014,
    ID_NOTIFY_AUTH_FRIEND_REQ = 1015, //对方同意了我(服务端主动推)
    ID_TEXT_CHAT_MSG_REQ      = 1017, //发文本消息
    ID_TEXT_CHAT_MSG_RSP      = 1018,
    ID_NOTIFY_TEXT_CHAT_MSG_REQ = 1019, //收到消息(含离线补投)
    ID_LOAD_CHAT_THREAD_REQ   = 1025, //拉好友列表/待处理申请
    ID_LOAD_CHAT_THREAD_RSP   = 1026,
    ID_LOAD_CHAT_MSG_REQ      = 1029, //拉历史消息
    ID_LOAD_CHAT_MSG_RSP      = 1030,
};

enum TipErr{
    TIP_SUCCESS = 0,
    TIP_EMAIL_ERR = 1,
    TIP_PWD_ERR = 2,
    TIP_CONFIRM_ERR = 3,
    TIP_PWD_CONFIRM = 4,
    TIP_VARIFY_ERR = 5,
    TIP_USER_ERR = 6
};

enum Modules{
    REGISTERMOD = 0,
    RESETMOD = 1,
    LOGIBMOD = 2,
};

enum ErrorCodes{
    SUCCESS = 0,
    ERR_JSON = 1, //JSON解析失败
    ERR_NETWORK = 2, //网络错误
};

enum ClickLbState{
    Normal = 0,
    Selected = 1
};

enum ChatUIMode{
    SearchMode,     //搜索
    ChatMode,       //聊天
    ContactMode,    //联系人模式
    SettingsMode,   //设置模式
};

enum ListItemType{
    CHAT_USER_ITEM,     // 聊天用户
    CONTACT_USER_ITEM,  // 联系人用户
    SEARCH_USER_ITEM,   // 搜索到的用户
    ADD_USER_TIP_ITEM,  // 提示添加用户
    INVALID_ITEM,       // 不可点击条目
    GROUP_TIP_ITEM,     // 分组提示条目
    LINE_ITEM,          // 分割线
    APPLY_FRIEND_ITEM,  // 好友申请
};

enum class ChatRole
{
    Self,
    Other
};



struct ServerInfo
{
    QString Host;
    QString Port;
    QString Token;
    int Uid;
};
extern QString gate_url_prefix;

#endif // GLOBAL_H
