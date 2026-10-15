#include "statewidget.h"
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QStyleOption>
#include <QPainter>

//     setProperty 只改数据, QSS 不会自动重算;
//     repolish(unpolish+polish) 才让样式表按新属性重新选规则;
//     update() 让控件重画。
void StateWidget::applyState(const QString &key)
{
    setProperty("state", key);
    repolish(this);
    update();
}

StateWidget::StateWidget(QWidget *parent)
    : QWidget{parent}, _curstate(ClickLbState::Normal)
{
    setCursor(Qt::PointingHandCursor); //小手
    AddRedPoint();
}

void StateWidget::SetState(QString normal, QString hover, QString press, QString select, QString select_hover, QString select_press)
{
    _normal = normal; _normal_hover = hover; _normal_press = press;
    _selected = select;_selected_hover = select_hover; _selected_press = select_press;
    applyState(_normal);
}

ClickLbState StateWidget::GetCurstate()
{
    return _curstate;
}

void StateWidget::ClearState()
{
    _curstate = ClickLbState::Normal;
    applyState(_normal);
}

void StateWidget::SetSelected(bool bselected)
{
    _curstate = bselected ? ClickLbState::Selected : ClickLbState::Normal;
    applyState(bselected ? _selected : _normal);
}

void StateWidget::AddRedPoint()
{
    _red_point = new QLabel();
    _red_point->setObjectName("red_point");
    _red_point->setAlignment(Qt::AlignCenter);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(_red_point);
    layout->setContentsMargins(0,0,0,0);
    this->setLayout(layout);

    _red_point->setVisible(false);
}

void StateWidget::ShowRedPoint(bool show)
{
    _red_point->setVisible(show);
}
// 自定义 QWidget 想让 QSS 的背景/边框/图片生效, 必须重写 paintEvent
void StateWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
// 按下: 直接进入"选中", 并显示"选中按下"的图
void StateWidget::mousePressEvent(QMouseEvent *ev)
{
    if(ev->button() == Qt::LeftButton) {
        _curstate = ClickLbState::Selected;
        applyState(_selected_press);
        return;
    }
    QWidget::mousePressEvent(ev);
}
// 抬起: 停在"选中悬停", 并发出 clicked() —— 只有抬起才发信号(和按钮一致)
void StateWidget::mouseReleaseEvent(QMouseEvent *ev)
{
    if(ev->button() == Qt::LeftButton) {
        applyState(_curstate == ClickLbState::Selected ? _selected_hover : _normal_hover);
        emit clicked();
        return;
    }
    QWidget::mouseReleaseEvent(ev);
}
// 鼠标进入: 按当前状态显示 hover 图
void StateWidget::enterEvent(QEvent *event)
{
    applyState(_curstate == ClickLbState::Normal ? _normal_hover : _selected_hover);
    QWidget::enterEvent(event);
}
// 鼠标离开: 回到 normal 或 selected
void StateWidget::leaveEvent(QEvent *event)
{
    applyState(_curstate == ClickLbState::Normal ? _normal : _selected);
    QWidget::leaveEvent(event);
}
