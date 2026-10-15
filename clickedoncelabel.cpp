#include "clickedoncelabel.h"

ClickedOnceLabel::ClickedOnceLabel(QWidget *parent) : QLabel(parent)
{
    // 手型光标: 让用户知道这东西能点
    this->setCursor(Qt::PointingHandCursor);
}

void ClickedOnceLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    // 为什么在 release 而不是 press 里发?
    //   按住鼠标拖出去再松手 = 用户反悔了, 不该触发 —— release 落在控件内才算点击。
    //   (Qt 的 QLabel 默认不处理鼠标, 我们这里只拦左键, 其他键交回基类)
    if (ev->button() == Qt::LeftButton) {
        emit clicked(this->text());
        return;
    }
    QLabel::mouseReleaseEvent(ev);
}
