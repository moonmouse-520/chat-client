#include "chatuserwid.h"
#include "ui_chatuserwid.h"

ChatUserWid::ChatUserWid(QWidget *parent)
    : ListItemBase(parent)
    , ui(new Ui::ChatUserWid)
{
    ui->setupUi(this);
    SetItemType(ListItemType::CHAT_USER_ITEM);
    // 红点要"叠"在头像右上角 -> 先把它从布局里摘出来(否则布局会把它摆到旁边/挤出可见区)
    if (ui->icon_wid->layout() != nullptr) {
        ui->icon_wid->layout()->removeWidget(ui->red_point);
    }
    ui->red_point->setParent(ui->icon_wid);
    ui->red_point->setFixedSize(10, 10);
    ui->red_point->move(30, 0);      // 40x40 头像的右上角
    ui->red_point->hide();
    ui->red_point->setText(QString());
    ui->time_lb->clear();
}

ChatUserWid::~ChatUserWid()
{
    delete ui;
}

void ChatUserWid::SetInfo(QString name, QString head, QString msg)
{
    _name = name;
    _head = head;
    _msg  = msg;

    QPixmap pixmap(_head);
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(),
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);
    ui->user_name_lb->setText(_name);   //名字
    ui->user_chat_lb->setText(_msg);    //最后一条消息
}

void ChatUserWid::ShowRedPoint(bool show)
{
    if (show) {
        ui->red_point->raise();      // 提到最上层, 防止被头像盖住
    }
    ui->red_point->setVisible(show);
}

void ChatUserWid::SetLastMsg(const QString& msg, const QString& when)
{
    _msg = msg;
    ui->user_chat_lb->setText(msg);

    if (!when.isEmpty()) {
        ui->time_lb->setText(when);        // 右上角显示这条消息的时间
    }
}
