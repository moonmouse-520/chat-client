#ifndef CONUSERITEM_H
#define CONUSERITEM_H

#include <QWidget>
#include <memory>
#include "listitembase.h"
#include "userdata.h"

namespace Ui {
class ConUserItem;
}

class ConUserItem : public ListItemBase
{
    Q_OBJECT
public:
    explicit ConUserItem(QWidget *parent = nullptr);
    ~ConUserItem();

    QSize sizeHint() const override;

    void SetInfo(std::shared_ptr<FriendInfo> friend_info);
    std::shared_ptr<FriendInfo> GetInfo();

private:
    Ui::ConUserItem *ui;
    std::shared_ptr<FriendInfo> _friend_info;
};

#endif // CONUSERITEM_H
