#ifndef CHATVIEW_H
#define CHATVIEW_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QTimer>
#include <QScrollBar>
class ChatView : public QWidget
{
    Q_OBJECT
public:
    explicit ChatView(QWidget *parent = nullptr);
    void appendChatItem(QWidget *item);     //尾插
    void prependChatItem(QWidget *item);    //头插(以后加载历史往上翻用)
    void insertChatItem(QWidget *before, QWidget *item);   // 中间插
    void removeAllItem();                                  // 清空
    QScrollBar* GetScrollBar() { return m_pScrollArea->verticalScrollBar(); }
    void prependChatItems(const QList<QWidget*>& items);     //插到顶部(保持顺序)
protected:
    bool eventFilter(QObject *o, QEvent *e) override;      // 鼠标进出: 显示/隐藏滚动条
    void paintEvent(QPaintEvent *event) override;          // 让 QSS 背景生效

signals:
    void sig_load_more();
private slots:
    void onVScrollBarMoved(int min, int max);

private:
    QScrollArea *m_pScrollArea = nullptr;
    bool isAppended = false;   // 刚刚"追加过消息"(用来决定要不要自动滚到底)
};

#endif // CHATVIEW_H
