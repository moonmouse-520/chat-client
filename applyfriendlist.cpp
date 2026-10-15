#include "applyfriendlist.h"


ApplyFriendList::ApplyFriendList(QWidget *parent) : QListWidget(parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    viewport()->installEventFilter(this);
}

bool ApplyFriendList::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == viewport()) {
        if (event->type() == QEvent::Enter) {
            setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        } else if (event->type() == QEvent::Leave) {
            setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
    }
    if (watched == viewport() && event->type() == QEvent::Wheel) {
        QWheelEvent *we = static_cast<QWheelEvent*>(event);
        const int steps = (we->angleDelta().y() / 8) / 15;
        verticalScrollBar()->setValue(verticalScrollBar()->value() - steps);
        return true;
    }
    return QListWidget::eventFilter(watched, event);
}
