#include "clickedlabel.h"

ClickedLabel::ClickedLabel(QWidget *parent) : QLabel(parent), _curstate(ClickLbState::Normal)
{
    this->setCursor(Qt::PointingHandCursor);
}

void ClickedLabel::mousePressEvent(QMouseEvent *ev)
{
    if(ev->button() == Qt::LeftButton) {
        //  注意: 状态的翻转发生在【按下】这一步。
        //   于是 mouseRelease 里 emit 出去的 _curstate 已经是"点完之后"的状态,
        //   收到信号的槽直接用就行, 不用自己再翻一次。
        if(_curstate == ClickLbState::Normal){
            _curstate = ClickLbState::Selected;
            setProperty("state", _selected_press);
        }else{
            _curstate = ClickLbState::Normal;
            setProperty("state", _normal_press);
        }
        repolish(this);
        update();
        return;
    }
    QLabel::mousePressEvent(ev);
}

void ClickedLabel::enterEvent(QEvent *event)
{
    setProperty("state", _curstate == ClickLbState::Normal ? _normal_hover : _selected_hover);
    repolish(this);
    update();
    QLabel::enterEvent(event);
}

void ClickedLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    if(ev->button() == Qt::LeftButton) {
        if(_curstate == ClickLbState::Normal){

            setProperty("state", _normal_hover);
        }else{
            setProperty("state", _selected_hover);
        }
        repolish(this);
        update();
        emit clicked(this->text(), _curstate);
        return;
    }
    QLabel::mouseReleaseEvent(ev);
}

void ClickedLabel::leaveEvent(QEvent *event)
{
    setProperty("state", _curstate == ClickLbState::Normal ? _normal : _selected);
    repolish(this);
    update();
    QLabel::leaveEvent(event);
}

void ClickedLabel::SetState(QString normal, QString hover, QString press, QString select, QString select_hover, QString select_press)
{
    _normal = normal;            _normal_hover = hover;    _normal_press = press;
    _selected = select;          _selected_hover = select_hover;  _selected_press = select_press;
    setProperty("state", normal);   // 初始状态
    repolish(this);
}

ClickLbState ClickedLabel::GetCurState()
{
    return _curstate;
}

// 代码主动设置选中态(用户没点它, 是程序帮他点亮的)
bool ClickedLabel::SetCurState(ClickLbState state)
{
    _curstate = state;

    if (_curstate == ClickLbState::Selected) {
        setProperty("state", _selected);
    }
    else if (_curstate == ClickLbState::Normal) {
        setProperty("state", _normal);
    }

    // setProperty 只是改了属性, Qt 不会自动重刷样式 —— repolish 才是"重新套一遍 qss"
    repolish(this);
    update();
    return true;
}

void ClickedLabel::ResetNormalState()
{
    SetCurState(ClickLbState::Normal);
}
