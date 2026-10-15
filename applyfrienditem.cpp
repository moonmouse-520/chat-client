#include "applyfrienditem.h"
#include "ui_applyfrienditem.h"

ApplyFriendItem::ApplyFriendItem(QWidget *parent)
    : ListItemBase(parent)
    , ui(new Ui::ApplyFriendItem)
{
    ui->setupUi(this);
    SetItemType(ListItemType::APPLY_FRIEND_ITEM);   //打上类型标记
    ui->addBtn->hide();
    ui->already_add_lb->hide();

    connect(ui->addBtn, &ClickedBtn::clicked, this, [this](){
        emit sig_auth_friend(_info);
    });
}

ApplyFriendItem::~ApplyFriendItem()
{
    delete ui;
}

void ApplyFriendItem::SetInfo(std::shared_ptr<ApplyInfo> info)
{
    _info = info;

    QPixmap pix(_info->_icon.isEmpty() ? QStringLiteral(":/res/head_1.jpg") : _info->_icon);
    ui->icon_lb->setPixmap(pix.scaled(ui->icon_lb->size(),Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);
    ui->user_name_lb->setText(_info->_name);
    ui->user_chat_lb->setText(_info->_desc.isEmpty() ? tr("请求添加你为好友") : _info->_desc);

    //status == 0 说明还没有处理  显示添加按钮, 否则显示已经添加
    showAddBtn(_info->_status == 0);
}

void ApplyFriendItem::showAddBtn(bool bshow)
{
    ui->addBtn->setVisible(bshow);
    ui->already_add_lb->setVisible(!bshow);
}
