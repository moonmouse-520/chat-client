#ifndef APPLYFRIENDITEM_H
#define APPLYFRIENDITEM_H

#include <QWidget>
#include "listitembase.h"
#include <memory>
#include "userdata.h"
namespace Ui {
class ApplyFriendItem;
}

class ApplyFriendItem :public ListItemBase
{
    Q_OBJECT

public:
    explicit ApplyFriendItem(QWidget *parent = nullptr);
    ~ApplyFriendItem();

    void SetInfo(std::shared_ptr<ApplyInfo> info);   // 填头像/名字/提示
    void showAddBtn(bool bshow);                    // 显示"添加"按钮 还是 "已添加"文字
    int  GetUid() { return _info ? _info->_uid : 0; }

    QSize sizeHint() const override { return QSize(250, 80); }
private:
    Ui::ApplyFriendItem *ui;
    std::shared_ptr<ApplyInfo> _info;
signals:
    // 点"添加"时发出去 ——  会用它弹出"认证"窗口
    void sig_auth_friend(std::shared_ptr<ApplyInfo> info);
};

#endif // APPLYFRIENDITEM_H
