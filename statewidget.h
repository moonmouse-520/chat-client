#ifndef STATEWIDGET_H
#define STATEWIDGET_H

#include <QWidget>
#include <QLabel>
#include "global.h"
class StateWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StateWidget(QWidget *parent = nullptr);
    void SetState(QString normal = "", QString hover = "", QString press = "",
                  QString select = "", QString select_hover = "", QString select_press = "");
    ClickLbState GetCurstate();
    void ClearState();          //回复未选中状态
    void SetSelected(bool bselected);         //直接设为选中状态

    void AddRedPoint();
    void ShowRedPoint(bool show = true);       // 显示/隐藏红点(有新申请时用)
    void applyState(const QString &key);
protected:
    void paintEvent(QPaintEvent *event) override;   // 让 QSS 背景/图片生效
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;
    void enterEvent(QEvent *event) override;           // 鼠标进入
    void leaveEvent(QEvent *event) override;           // 鼠标离开
private:
    QString _normal, _normal_hover, _normal_press;
    QString _selected, _selected_hover, _selected_press;

    ClickLbState _curstate;
    QLabel *_red_point = nullptr;

signals:
    void clicked(void);  // 和 ClickedLabel 一样: 抬起时发一次
};

#endif // STATEWIDGET_H
