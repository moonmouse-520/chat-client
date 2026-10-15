#include "findsuccessdlg.h"
#include "ui_findsuccessdlg.h"
#include <QJsonObject>
#include <QJsonDocument>
#include "applyfriend.h"
FindSuccessDlg::FindSuccessDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FindSuccessDlg)
    , _parent(parent)
{
    ui->setupUi(this);
    setWindowTitle(tr("添加"));
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    ui->add_friend_btn->SetState("normal", "hover", "press");
    setModal(true);
}

FindSuccessDlg::~FindSuccessDlg()
{
    delete ui;
}

void FindSuccessDlg::SetSearchInfo(std::shared_ptr<SearchInfo> si)
{
    _si = si;
    ui->name_lb->setText(_si->_name);
    // 头像: 服务端给的 icon 是资源路径(如 ":/res/head_1.jpg"); 空的话给个默认
    QPixmap pix(_si->_icon.isEmpty() ? QStringLiteral(":/res/head_1.jpg") : si->_icon);
    ui->head_lb->setPixmap(pix.scaled(ui->head_lb->size(),Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->head_lb->setScaledContents(true);
}

void FindSuccessDlg::on_add_friend_btn_clicked()
{
    if (_si == nullptr) {
        return;
    }

    this->hide();

    auto *apply_friend = new ApplyFriend(_parent);
    apply_friend->SetSearchInfo(_si);
    apply_friend->setModal(true);
    apply_friend->show();
}

