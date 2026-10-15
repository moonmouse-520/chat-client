#ifndef CLICKEDLABEL_H
#define CLICKEDLABEL_H

#include <QLabel>
#include "global.h"
#include <QMouseEvent>
class ClickedLabel : public QLabel
{
    Q_OBJECT
public:
    ClickedLabel(QWidget *parent = nullptr);
    virtual void mousePressEvent(QMouseEvent *ev) override;
    virtual void enterEvent(QEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *ev) override;
    virtual void leaveEvent(QEvent *event) override;

    void SetState(QString normal = "", QString hover="",QString press="",
                  QString select = "", QString select_hover = "", QString select_press = "");
    ClickLbState GetCurState();

    // 从代码里直接把它切成"选中/未选中"
    //    (用户点"同学"这个推荐标签时, 程序要主动把它点亮成绿色)
    bool SetCurState(ClickLbState state);

    //  变回未选中 —— 其实就是 SetCurState(Normal) 的语法糖,
    //    用途: 用户把某个标签从"已选"里删掉时, 推荐栏里那个同名标签也要跟着变灰
    void ResetNormalState();

private:
    QString _normal;
    QString _normal_hover;
    QString _normal_press;

    QString _selected;
    QString _selected_hover;
    QString _selected_press;

    ClickLbState _curstate;
signals:
    //   现在把"我是谁"(文字)和"我点完之后是什么状态"一起发出去 ——
    //   因为标签是代码 new 出来的, 弹窗没有它们的指针数组, 只能靠信号把身份带出来。
    //   注意发放时机: mousePress 里已经翻转了 _curstate, 所以发出去的是"点击后"的状态。
    void clicked(QString, ClickLbState);
};

#endif // CLICKEDLABEL_H
