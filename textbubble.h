#ifndef TEXTBUBBLE_H
#define TEXTBUBBLE_H
//文本气泡, 宽高自适应
#include "bubbleframe.h"
#include <QTextEdit>
class TextBubble : public BubbleFrame
{
    Q_OBJECT
public:
    TextBubble(ChatRole role, const QString &text, QWidget *parent = nullptr);
protected:
    bool eventFilter(QObject *o, QEvent *e) override;
private:
    void setPlainText(const QString &text);//顺便算出气泡的最大宽度
    void adjustTextHeight();               //换行后按实际行数定高
    void initStyleSheet();
private:
    QTextEdit *m_pTextEdit = nullptr;
};

#endif // TEXTBUBBLE_H
