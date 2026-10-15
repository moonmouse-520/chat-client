#ifndef BUBBLEFRAME_H
#define BUBBLEFRAME_H

#include <QFrame>
#include <QHBoxLayout>
#include "global.h"

// 气泡基类: 自己画圆角矩形 + 指向头像的小三角
//   Self  -> 绿色(158,234,106), 三角在右边
//   Other -> 白色,                三角在左边
class BubbleFrame : public QFrame
{
    Q_OBJECT
public:
    BubbleFrame(ChatRole role, QWidget *parent = nullptr);

    void setMargin(int margin);
    void setWidget(QWidget *w);     //往气泡里面放内容
protected:
    void paintEvent(QPaintEvent *e) override;
private:
    QHBoxLayout *m_pHLayout = nullptr;
    ChatRole m_role;
    int m_margin;
};

#endif // BUBBLEFRAME_H
