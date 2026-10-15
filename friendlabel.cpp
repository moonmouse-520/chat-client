#include "friendlabel.h"
#include "ui_friendlabel.h"
#include "clickedlabel.h"
#include <QFontMetrics>

FriendLabel::FriendLabel(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::FriendLabel),
    _width(0),
    _height(0)
{
    ui->setupUi(this);

    // × 按钮的六态颜色都交给 qss(靠 state 属性选), 这里只把状态名登记进去
    ui->close_lb->SetState("normal", "hover", "pressed",
                           "selected_normal", "selected_hover", "selected_pressed");

    //  新信号是两个参数 clicked(QString, ClickLbState),
    //   而 slot_close() 没有参数 —— Qt 允许"槽的参数比信号少", 多余的实参直接丢掉。
    connect(ui->close_lb, &ClickedLabel::clicked, this, &FriendLabel::slot_close);
}

FriendLabel::~FriendLabel()
{
    delete ui;
}

void FriendLabel::SetText(QString text)
{
    _text = text;
    ui->tip_lb->setText(_text);

    // adjustSize(): 让 tip_lb 按文字自动算大小, 这样下面量出来的宽高才是真的
    ui->tip_lb->adjustSize();

    // 用字体度量算文字有多宽/多高 —— 这是"手工排版"必须自己做的一步
    QFontMetrics fontMetrics(ui->tip_lb->font());
    int textHeight = fontMetrics.height();

    // 自己多宽 = 文字宽 + ×按钮宽(15) + 5 的余量; 高 = 文字高 + 2
    this->setFixedWidth(ui->tip_lb->width() + ui->close_lb->width() + 5);
    this->setFixedHeight(textHeight + 2);

    // 缓存下来给弹窗排版用
    _width = this->width();
    _height = this->height();
}

int FriendLabel::Width()
{
    return _width;
}

int FriendLabel::Height()
{
    return _height;
}

QString FriendLabel::Text()
{
    return _text;
}

void FriendLabel::slot_close()
{
    emit sig_close(_text);
}
