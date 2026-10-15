#include "searchlist.h"
#include "adduseritem.h"
#include "tcpmgr.h"
#include "loadingdlg.h"
#include "customizeedit.h"
#include "usermgr.h"
#include <QJsonDocument>
#include <QMessageBox>
#include "findfaildlg.h"
#include "findsuccessdlg.h"
SearchList::SearchList(QWidget *parent)
{
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //事件过滤器装在 viewport() 上 —— 第 19 集踩过: 装在 this 上收不到滚轮/进出事件
    this->viewport()->installEventFilter(this);
    addTipItem();

    connect(this, &QListWidget::itemClicked,this, &SearchList::slot_item_clicked);

    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_user_search,
            this, &SearchList::slot_user_search);

    // 1010 回来了: 用弹窗告诉用户结果
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_add_friend_rsp, this, [this](int err) {
        QString tip;
        switch (err) {
        case 0:    tip = tr("好友申请已发出"); break;
        case 1014: tip = tr("已经申请过了, 别急"); break;      // ApplyDuplicate
        case 1015: tip = tr("你们已经是好友了"); break;        // AlreadyFriend
        case 1010: tip = tr("登录状态失效, 请重新登录"); break; // TokenInvalid
        default:   tip = tr("申请失败, 错误码 %1").arg(err); break;
        }
        QMessageBox::information(this, tr("提示"), tip);
    });
}

// 鼠标进出: 显隐滚动条; 滚轮: 自己滚
//   (比 ChatUserList 少了"滚到底发信号"那段, 因为搜索列表不需要加载更多)
bool SearchList::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == this->viewport()) {
        if(event->type() == QEvent::Enter) {
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        }else if(event->type() == QEvent::Leave){
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
    }

    if (watched == this->viewport() && event->type() == QEvent::Wheel) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        const int numSteps = (wheelEvent->angleDelta().y() / 8) / 15;
        this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() - numSteps);
        return true;     // 自己处理了, 别让列表再滚一次
    }
    return QListWidget::eventFilter(watched, event);
}

// 加两行: 一条灰色占位条 + 一行"查找 uid/name"
void SearchList::addTipItem()
{
    auto *invalid_item = new QWidget();
    invalid_item->setObjectName("invalid_item");    // QSS 里 #invalid_item 给它灰底
    auto *item_tmp = new QListWidgetItem;
    item_tmp->setSizeHint(QSize(250, 10));          // 高度很小, 看起来像一条分隔条
    this->addItem(item_tmp);
    this->setItemWidget(item_tmp, invalid_item);
    //  去掉"可选中"标志 —— 否则鼠标划过/点它会高亮变蓝, 看着像能点
    item_tmp->setFlags(item_tmp->flags() & ~Qt::ItemIsSelectable);

    auto *add_user_item = new AddUserItem();
    auto *item = new QListWidgetItem;
    item->setSizeHint(add_user_item->sizeHint());   // 70 高
    this->addItem(item);
    this->setItemWidget(item, add_user_item);
}

void SearchList::SetSearchEdit(QWidget* edit)
{
    _search_edit = edit;
}

void SearchList::CloseFindDlg()
{
    if(_find_dlg) {
        // 关掉并且回收: 弹窗是 new 出来的(自己的子控件), 只 hide 不删 -> 每搜一次漏一个
        // 用 deleteLater 而不是 delete: 此刻可能正在处理鼠标事件, 立刻 delete 会把事件目标一起删掉
        _find_dlg->hide();
        _find_dlg->deleteLater();
        _find_dlg = nullptr;
    }
}

void SearchList::waitPending(bool pending){
    if (pending) {
        _loading_dlg = new LoadingDlg(this, tr("搜索中..."));
        _loading_dlg->setModal(true);
        _loading_dlg->show();
        _send_pending = true;
    }else{
        if(_loading_dlg) {
            _loading_dlg->hide();
            _loading_dlg->deleteLater();
            _loading_dlg = nullptr;
        }
        _send_pending = false;
    }
}

// 点列表里的一行: 靠 ListItemBase 的"类型标记"判断点到了什么
void SearchList::slot_item_clicked(QListWidgetItem* item){
    QWidget* wid= this->itemWidget(item);
    auto* base = qobject_cast<ListItemBase*>(wid);

    if(!base) {
        return;
    }

    if(base->GetItemType() == ListItemType::INVALID_ITEM) {
        return;
    }

    if (base->GetItemType() == ListItemType::ADD_USER_TIP_ITEM) {
        if (_send_pending || _search_edit == nullptr) {
            return;                                // 正在搜 / 还没拿到搜索框
        }

        auto* edit = dynamic_cast<CustomizeEdit*>(_search_edit);

        if (!edit) {
            return;
        }

        const QString key = edit->text().trimmed();
        if (key.isEmpty()) {
            return;
        }

        waitPending(true);  // 弹"搜索中"遮罩

        QJsonObject req;
        req["uid"]       = UserMgr::GetInstance()->GetUid();  // 服务端只用来打日志
        req["searchkey"] = key;                               // 我们服务端要的是这个
        emit TcpMgr::GetInstance()->sig_send_data(
            ReqId::ID_SEARCH_USER_REQ,
            QString(QJsonDocument(req).toJson(QJsonDocument::Compact)));
        return;
    }
}

// 结果回来了
void SearchList::slot_user_search(std::shared_ptr<SearchInfo> si)
{
    waitPending(false);
    CloseFindDlg();
    if(si == nullptr) {
        _find_dlg = new FindFailDlg(this);
    }else{
        if(si->_uid == UserMgr::GetInstance()->GetUid()) {
            return;
        }

        _find_dlg = new FindSuccessDlg(this);
        static_cast<FindSuccessDlg*>(_find_dlg)->SetSearchInfo(si);
    }
    _find_dlg->show();
}