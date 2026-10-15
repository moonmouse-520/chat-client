#ifndef CLICKEDONCELABEL_H
#define CLICKEDONCELABEL_H

#include <QLabel>
#include <QMouseEvent>

 // ClickedOnceLabel —— 点一次就发信号"的标签
class ClickedOnceLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ClickedOnceLabel(QWidget *parent = nullptr);

    virtual void mouseReleaseEvent(QMouseEvent *ev) override;

signals:
    // 把 label 上显示的文字原样发出去 —— 调用方不用再去问 "你点了哪个"
    void clicked(QString);
};

#endif // CLICKEDONCELABEL_H
