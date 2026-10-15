#include "friendinfopage.h"
#include "ui_friendinfopage.h"      // uic 从 friendinfopage.ui 生成的

// ============================================================
// 构造函数
//   FriendInfoPage 是"好友资料页"整页 —— 它和 ChatPage(会话页)、
//   ApplyFriendPage(申请页) 一样, 都放在 ChatDialog 的 stackedWidget 里,
//   同一时刻只显示一个。
// ============================================================
FriendInfoPage::FriendInfoPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FriendInfoPage)
{
    ui->setupUi(this);

    // "发消息"按钮的三态。
    // ⭐ SetState 里做的事是 setProperty("state", 字符串) + repolish + update,
    //    于是 QSS 里就能用属性选择器分别画三种样子:
    //        #msg_chat[state='normal'] { ... }
    //        #msg_chat[state='hover']  { ... }
    //        #msg_chat[state='press']  { ... }
    //    这就是 StateWidget / ClickedBtn 那一套"用 QSS 换图"的通用做法。
    ui->msg_chat->SetState("normal", "hover", "press");
}

FriendInfoPage::~FriendInfoPage()
{
    delete ui;      // ui 是自己 new 的, 自己删
}

// ============================================================
// SetInfo(): 拿一条好友数据刷新整页
//   同样是"数据在外面(UserMgr), 页面只负责显示"的思路:
//   页面自己不查库、不发请求, 谁想显示谁调 SetInfo()。
// ============================================================
void FriendInfoPage::SetInfo(std::shared_ptr<FriendInfo> friend_info)
{
    _friend_info = friend_info;     // 留一份: 点"发消息"时要把它带出去
    if (!_friend_info) {
        return;
    }

    // 大标题 = 用户名(不是备注) —— 资料页里要显示"这个人本身叫什么"
    ui->name_lb->setText(_friend_info->_name);

    // 下面几项都是"可能为空"的字段, 空的时候给个占位提示,
    // 免得界面上出现一行空白让人以为是 bug。
    // tr() 是给翻译用的接口: 字符串包在 tr() 里, 以后要做多语言直接换翻译文件即可。
    QString nick = _friend_info->_nick.isEmpty() ? tr("(未设置)") : _friend_info->_nick;
    ui->nick_lb->setText(tr("昵称: %1").arg(nick));     // %1 是占位符, arg() 填进去

    ui->bak_lb->setText(_friend_info->_back.isEmpty() ? tr("(无)") : _friend_info->_back);

    QString desc = _friend_info->_desc.isEmpty() ? tr("这个人很懒, 什么都没留下") : _friend_info->_desc;
    ui->desc_lb->setText(desc);

    // 服务端约定: sex = 0 男, 1 女(看 LogicSystem 里注册时写库的值)
    ui->sex_lb->setText(_friend_info->_sex == 0 ? tr("男") : tr("女"));

    // 头像: 和联系人行同一套写法(保持长宽比 + 平滑缩放)
    QPixmap pix(_friend_info->_icon);
    if (!pix.isNull()) {
        ui->icon_lb->setPixmap(pix.scaled(ui->icon_lb->size(),
                                          Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation));
    }
}

// ============================================================
// "发消息"按钮的槽
//   ⭐ 名字是 Qt 的"自动连接"约定: on_<控件objectName>_<信号名>()
//      .ui 里这个按钮的 objectName 是 msg_chat, 信号是 clicked,
//      于是 uic 生成的 setupUi() 里会自动 connect(msg_chat, clicked, this, on_msg_chat_clicked),
//      一行 connect 都不用自己写。
//      代价: 改了 .ui 里的 objectName 而忘了改函数名 —— 编译能过, 但点了没反应。
// ============================================================
void FriendInfoPage::on_msg_chat_clicked()
{
    // 不自己切页面, 只把"要跟这个人聊天"发出去(谁接到谁处理, 目前是 ChatDialog)
    if (_friend_info) {
        emit sig_jump_chat_item(_friend_info);
    }
}
