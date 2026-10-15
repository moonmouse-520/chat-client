#include "chatuserlist.h"

ChatUserList::ChatUserList(QWidget *parent) : QListWidget(parent)
{
    // 默认两个滚动条都关掉, 鼠标进来才显示(平时看着干净)
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // ★ 事件过滤器要装在 viewport() 上, 不是装在列表本身:
    //   鼠标的"滚轮/进入/离开"事件是发给显示区域(viewport)的,
    //   装在 this 上就收不到 —— 这是最常见的"怎么写都没反应"的原因
    this->viewport()->installEventFilter(this);
}

bool ChatUserList::eventFilter(QObject *watched, QEvent *event)
{
    // ---- 1) 鼠标进/出: 显示/隐藏竖直滚动条 ----
    if (watched == this->viewport()) {
        if (event->type() == QEvent::Enter) {
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);   // 鼠标进来: 需要才显示
        } else if (event->type() == QEvent::Leave) {
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);  // 鼠标离开: 藏起来
        }
    }

    // ---- 2) 滚轮: 自己滚动 + 判断是否到底 ----
    if (watched == this->viewport() && event->type() == QEvent::Wheel) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        const int numDegrees = wheelEvent->angleDelta().y() / 8;   // 一个"格"是 120, 除以 8 = 15
        const int numSteps   = numDegrees / 15;                    // 换算成"几步"

        QScrollBar *scrollBar = this->verticalScrollBar();
        scrollBar->setValue(scrollBar->value() - numSteps);

        // 到达底部(还能往下滚的空间 <= 0) -> 通知外面加载下一页
        if (scrollBar->maximum() - scrollBar->value() <= 0) {
            qDebug() << "[chatuserlist] load more";
            emit sig_loading_chat_user();      //
        }

        return true;   // 事件到此为止, 不再传给 QListWidget(否则会滚两次)
    }

    return QListWidget::eventFilter(watched, event);
}
