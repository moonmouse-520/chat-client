#include "conuseritem.h"
#include "ui_conuseritem.h"     // ★ uic 从 conuseritem.ui 生成的界面代码(里面是 setupUi 和 icon_lb/name_lb)

ConUserItem::ConUserItem(QWidget *parent)
    : ListItemBase(parent)
    , ui(new Ui::ConUserItem)
{
    ui->setupUi(this);

    SetItemType(ListItemType::CONTACT_USER_ITEM);
}

// ui 是自己 new 的, 必须自己 delete; ui 里的控件是 setupUi 建在 this 上的,
// 会跟着 this 一起被 Qt 的父子机制回收, 不用手动删
ConUserItem::~ConUserItem()
{
    delete ui;
}

// ============================================================
// sizeHint(): 告诉 QListWidget "我这一行该多高"
//   必须和 conuseritem.ui 里那个顶层控件的 maximumHeight(70) 一致。
//     列表每行的实际高度 = item->setSizeHint(...) 的值(见 ContactUserList::LoadFriendList),
//     而那个值又来自这里; 两边不一致的话, 要么行与行之间留缝,
//     要么控件被列表"压扁"(控件比行高, 底部被裁掉)。
// ============================================================
QSize ConUserItem::sizeHint() const
{
    return QSize(250, 70);      // 宽度 250 = 左栏宽度; 高度 70 = .ui 里设的最大高度
}

// ============================================================
// SetInfo(): 把一条好友数据"灌"到这一行的界面上
//   参数用 shared_ptr 而不是拷贝: 好友数据在 UserMgr 里只有一份,
//   这里只是"看一眼", 不需要复制; 同时自己留一份 _friend_info,
//   点行的时候要把它随信号发出去。
// ============================================================
void ConUserItem::SetInfo(std::shared_ptr<FriendInfo> friend_info)
{
    _friend_info = friend_info;
    if (!_friend_info) {            // 防空: 传进来空指针就直接不动界面
        return;
    }

    // 显示名字: 优先备注 -> 昵称 -> 用户名(这个优先级写死在 FriendInfo::GetShowName() 里)
    ui->name_lb->setText(_friend_info->GetShowName());

    // 头像: 图片路径来自服务端(资源路径 :/res/xxx 或本地文件路径)
    QPixmap pix(_friend_info->_icon);
    if (!pix.isNull()) {
        //  必须 scaled(): 好友头像原图大小不一, 不缩放会把这一行撑变形。
        //    KeepAspectRatio = 保持长宽比(不会拉扁), 空出来的部分留白;
        //    SmoothTransformation = 平滑缩放(默认是最近邻, 缩小时会很难看)。
        ui->icon_lb->setPixmap(pix.scaled(ui->icon_lb->size(),
                                          Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation));
    }
}

// 取这一行对应的好友数据 —— 点行时用它发 sig_switch_friend_info_page()
std::shared_ptr<FriendInfo> ConUserItem::GetInfo()
{
    return _friend_info;
}
