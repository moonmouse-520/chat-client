#include "chatview.h"
#include <QScrollBar>
#include <QTimer>
#include <QEvent>
#include <QStyleOption>
#include <QPainter>
#include <QDebug>
ChatView::ChatView(QWidget *parent)
    : QWidget{parent}, isAppended(false)
{
    QVBoxLayout *pMainLayout = new QVBoxLayout();
    this->setLayout(pMainLayout);
    pMainLayout->setContentsMargins(0, 0, 0, 0);

    m_pScrollArea = new QScrollArea();
    m_pScrollArea->setObjectName("chat_area");  // QSS 里 #chat_area 选中它
    pMainLayout->addWidget(m_pScrollArea);

    //内容层: 所有消息的父控件
    QWidget *w = new QWidget(this);
    w->setObjectName("chat_bg");
    w->setAutoFillBackground(true);
    QVBoxLayout *pContentLayout = new QVBoxLayout();
    //     垫一个能吃掉 100000 拉伸比的空 widget,
    //     消息才会"从顶部往下堆", 而不是被均匀撑满整个高度
    pContentLayout->addWidget(new QWidget(), 100000);
    w->setLayout(pContentLayout);
    m_pScrollArea->setWidget(w);

    // 悬浮滚动条: 先藏掉原生的, 再把 scrollbar 对象单独摆到右侧
    m_pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QScrollBar *pVScrollBar = m_pScrollArea->verticalScrollBar();
    // 内容高度变化(rangeChanged)时会通知我们 —— 用来自动滚到底
    connect(pVScrollBar, &QScrollBar::rangeChanged,this, &ChatView::onVScrollBarMoved);

    QHBoxLayout *pScrollBarLayout = new QHBoxLayout();
    pScrollBarLayout->addWidget(pVScrollBar, 0 , Qt::AlignRight);//靠右悬浮
    pScrollBarLayout->setContentsMargins(0,0,0,0);
    m_pScrollArea->setLayout(pScrollBarLayout);
    pVScrollBar->setHidden(true);

    m_pScrollArea->setWidgetResizable(true);    // 让内容层随滚动区宽度变化
    m_pScrollArea->installEventFilter(this);    // 鼠标进出要用

    connect(pVScrollBar, &QScrollBar::valueChanged, this, [this](int v) {
        if (v == 0 && m_pScrollArea->verticalScrollBar()->maximum() > 0) {
            emit sig_load_more();            //已经在最上面 -> 请求更老的
        }
    });
}

// 追加一条消息: 插在"占位 widget"【前面】
void ChatView::appendChatItem(QWidget *item)
{
    if (item == nullptr) {
        return;
    }
    QVBoxLayout *v1 = qobject_cast<QVBoxLayout *>(m_pScrollArea->widget()->layout());
    if (v1 == nullptr) {
        qDebug() << "[chatview] layout 不是 QVBoxLayout, 无法追加";
        return;
    }

    v1->insertWidget(v1->count(), item);
    isAppended = true;// 标记"刚追加过", 好让 onVScrollBarMoved 把视图拉到底
}

void ChatView::prependChatItem(QWidget *item)
{
    Q_UNUSED(item);
}

void ChatView::insertChatItem(QWidget *before, QWidget *item)
{
    Q_UNUSED(before);
    Q_UNUSED(item);
}

// 清空所有消息(保留最后那个占位 widget)
void ChatView::removeAllItem()
{
    QWidget* content = m_pScrollArea ? m_pScrollArea->widget() : nullptr;
    if (content == nullptr) {
        return;
    }
    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(content->layout());
    if (layout == nullptr) {
        return;
    }

    // 第 0 个 item 是构造函数里加的那个 100000 的 spacer(负责把消息顶到上面) -> 必须保留
    // 所以从最后一个开始删, 删到只剩它为止
    for (int i = layout->count() - 1; i >= 1; --i) {
        QLayoutItem* it = layout->takeAt(i);
        if (it == nullptr) {
            continue;
        }
        if (QWidget* w = it->widget()) {
            w->deleteLater();          // 安全删除; 不要 delete, 也不要 setParent(nullptr)
        }
        delete it;                     // takeAt 出来的 item 必须自己删
    }
}

void ChatView::prependChatItems(const QList<QWidget *> &items)
{
    QWidget* content = m_pScrollArea ? m_pScrollArea->widget() : nullptr;
    if (content == nullptr || content->layout() == nullptr) {
        return;
    }
    QVBoxLayout *vl = qobject_cast<QVBoxLayout*>(m_pScrollArea->widget()->layout());
    if(vl == nullptr) return;
    for(int i = 0; i<items.size();i++) {
        vl->insertWidget(i+1, items[i]);
    }
}

// 鼠标移进滚动区: 有内容才显示滚动条; 移出就藏起来
bool ChatView::eventFilter(QObject *o, QEvent *e)
{
    if(e->type() == QEvent::Enter && o == m_pScrollArea) {
        // maximum()==0 说明内容没超过一屏 -> 不显示
        m_pScrollArea->verticalScrollBar()->setHidden(
            m_pScrollArea->verticalScrollBar()->maximum() == 0);
    }else if((e->type() == QEvent::Leave && o == m_pScrollArea)){
        m_pScrollArea->verticalScrollBar()->setHidden(true);
    }
    return QWidget::eventFilter(o, e);
}

// 自定义 QWidget 想让 QSS 背景生效, 必须重写它(和 ListItemBase/ChatPage 一个道理)
void ChatView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

// ------------------------------------------------------------
// 内容高度变了
//   刚追加过消息 -> 自动滚到底(不然新消息在屏幕外看不见)
//     为什么要 isAppended + 500ms 防抖: rangeChanged 在布局过程中会连着触发好几次;
//     而且用户正在往上翻历史时不该被强行拉到底。
// ------------------------------------------------------------
void ChatView::onVScrollBarMoved(int min, int max)
{
    Q_UNUSED(min);
    Q_UNUSED(max);
    if (isAppended) {
        QScrollBar *pVScrollBar = m_pScrollArea->verticalScrollBar();
        pVScrollBar->setSliderPosition(pVScrollBar->maximum());   // 拉到底
        // 带 context 对象(this): 万一 500ms 内这个控件被销毁了, 回调直接不执行
        // (裸捕获 this 时, 这里就会写到一个已经释放的对象上)
        QTimer::singleShot(500, this, [this]() {                   // 500ms 后复位标记
            isAppended = false;
        });
    }
}
