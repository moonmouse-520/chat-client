#include "contactuserlist.h"
#include "conuseritem.h"
#include "usermgr.h"
#include <QPixmap>

// ============================================================
// 构造函数
//   类继承 QListWidget: 列表的滚动、选中、item 管理全部由 Qt 负责,
//   我们只负责"往里塞什么样的行"和"点行之后干什么"。
// ============================================================
ContactUserList::ContactUserList(QWidget *parent)
    : QListWidget(parent)
{
    // 左栏总共就 250 宽, 出现横向滚动条既难看也没意义 -> 永久关掉
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //  用 Qt5 的"函数指针"connect 语法(不是老式 SIGNAL/SLOT 宏):
    //    编译期就能检查签名是否匹配, 写错了编译不过, 而不是运行时静默失败。
    //    itemClicked 是点"已选中项"才发(第一次点只选中), 这正是我们要的行为。
    connect(this, &QListWidget::itemClicked, this, &ContactUserList::slot_item_clicked);
}

// ============================================================
// LoadFriendList(): 按 UserMgr 里的好友列表"全量重灌"左栏
//   什么时候调? 每次切到"联系人"模式都调一次(见 ChatDialog::slot_side_contact)——
//   这样刚同意的好友申请、刚收到的好友列表更新, 都能立刻反映出来。
//   数据量只有几十条, 重灌的成本可以忽略; 比"增量维护"少一堆边界 case。
// ============================================================
void ContactUserList::LoadFriendList()
{
    // ★ clear() 会把每个 QListWidgetItem 一起 delete。
    //   而 setItemWidget() 贴上去的控件是"挂"在这个 item 上的(item 被删时
    //   控件也会被删, 或者至少是安全的) —— 所以这里不会泄漏, 更不会留下野指针。
    //   ⛔ 千万别自己写 for (auto* w : xxx) delete w; 去删 setItemWidget 的控件:
    //      列表内部的 item<->widget 表还存着那个地址, 下次重绘/清空就会
    //      "释放已释放的对象", 表现为关闭窗口时莫名闪退(0xc0000005)。
    this->clear();

    auto friend_list = UserMgr::GetInstance()->GetFriendList();
    for (auto &friend_info : friend_list) {

        //  先造"行控件"（它就是一行界面: 头像 + 名字）
        auto *item_wid = new ConUserItem();
        item_wid->SetInfo(friend_info);

        //  再造"格子"
        QListWidgetItem *item = new QListWidgetItem;

        //  setSizeHint 必须在 addItem 之前设好: 列表是拿 item 的 sizeHint 当行高用的
        item->setSizeHint(item_wid->sizeHint());

        this->addItem(item);
        // 3) 把控件"贴"到格子上 —— 列表从此替我们管理它的位置与可见性
        this->setItemWidget(item, item_wid);
    }
}

// ============================================================
// 点中一行 -> 通知外面"要显示这个人的资料页"
//   这里【不】直接操作右侧页面: 列表控件不认识 ChatDialog,
//   它只负责 emit 一个带数据的信号, 由 ChatDialog 决定怎么反应(解耦)。
// ============================================================
void ContactUserList::slot_item_clicked(QListWidgetItem *item)
{
    // 用 qobject_cast 而不是 static_cast:
    //   itemWidget() 返回的是 QWidget*, 正常情况下一定是 ConUserItem,
    //   但万一类型不对, qobject_cast 会给 nullptr, 我们可以安全地 return,
    //   而不是拿着错误指针硬转(后果是随机崩溃)。
    auto *con_item = qobject_cast<ConUserItem*>(this->itemWidget(item));
    if (con_item == nullptr) {
        return;
    }

    // 把这一行的好友数据原样发出去(shared_ptr 拷贝, 引用计数 +1, 不会失效)
    emit sig_switch_friend_info_page(con_item->GetInfo());
}
