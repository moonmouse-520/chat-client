#include "bubbleframe.h"
#include <QPainter>
#include <QBrush>
const int WIDTH_SANJIAO = 8; // 小三角的宽度(像素) —— 画气泡时要给这一条留出位置

BubbleFrame::BubbleFrame(ChatRole role, QWidget *parent)
    : QFrame(parent)
    , m_role(role)
    , m_margin(3)
{
    m_pHLayout = new QHBoxLayout();
    // 给三角留位置: 自己的三角在右边 -> 右边多留 WIDTH_SANJIAO; 别人的在左边 -> 左边多留
    if(m_role == ChatRole::Self) {
        m_pHLayout->setContentsMargins(m_margin, m_margin, WIDTH_SANJIAO + m_margin, m_margin);
    }else{
        m_pHLayout->setContentsMargins(WIDTH_SANJIAO+m_margin,m_margin,m_margin,m_margin);
    }
    this->setLayout(m_pHLayout);
}

void BubbleFrame::setMargin(int margin)
{
    m_margin = margin;
}

void BubbleFrame::setWidget(QWidget *w)
{
    if(m_pHLayout->count() > 0) { // 只允许放一个内容
        return;
    }
    m_pHLayout->addWidget(w);
}

// 自己画气泡: 圆角矩形 + 一个小三角
void BubbleFrame::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setPen(Qt::NoPen);        // 不画轮廓线

    if (m_role == ChatRole::Other) {
        // ---- 别人的: 白色, 三角在左边 ----
        painter.setBrush(QBrush(QColor(Qt::white)));            //设画刷(填充色)
        QRect bk_rect = QRect(WIDTH_SANJIAO, 0, this->width() - WIDTH_SANJIAO, this->height());
        painter.drawRoundedRect(bk_rect, 5, 5);

        QPointF points[3] = {
            QPointF(bk_rect.x(), 12),
            QPointF(bk_rect.x(), 10 + WIDTH_SANJIAO + 2),
            QPointF(bk_rect.x() - WIDTH_SANJIAO, 10 + WIDTH_SANJIAO - WIDTH_SANJIAO / 2),
        };
        painter.drawPolygon(points, 3);
    } else {
        // ---- 自己的: 绿色(158,234,106), 三角在右边 ----
        painter.setBrush(QBrush(QColor(158, 234, 106)));
        QRect bk_rect = QRect(0, 0, this->width() - WIDTH_SANJIAO, this->height());
        painter.drawRoundedRect(bk_rect, 5, 5);

        QPointF points[3] = {
            QPointF(bk_rect.x() + bk_rect.width(), 12),
            QPointF(bk_rect.x() + bk_rect.width(), 12 + WIDTH_SANJIAO + 2),
            QPointF(bk_rect.x() + bk_rect.width() + WIDTH_SANJIAO,
                    10 + WIDTH_SANJIAO - WIDTH_SANJIAO / 2),
        };
        painter.drawPolygon(points, 3);
    }

    return QFrame::paintEvent(e);       // 保留: 让基类/QSS 该做的事继续
}