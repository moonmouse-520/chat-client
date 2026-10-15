#ifndef CHATITEMBASE_H
#define CHATITEMBASE_H

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include "global.h"
class ChatItemBase : public QWidget
{
    Q_OBJECT
public:
    explicit ChatItemBase(ChatRole role, QWidget *parent = nullptr);

    void setUserName(const QString& name);
    void setUserIcon(const QPixmap& icon);
    void setWidget(QWidget *w);
signals:

private:
    ChatRole m_role;
    QLabel *m_pNameLabel = nullptr;
    QLabel *m_pIconLabel = nullptr;
    QWidget *m_pBubble    = nullptr;   // 占位用的空 widget, 后面被 setWidget 替换掉
};

#endif // CHATITEMBASE_H
