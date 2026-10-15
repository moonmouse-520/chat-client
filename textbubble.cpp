#include "textbubble.h"
#include <QScrollBar>
#include <QTextBlock>
#include <QTextDocument>
#include <QFontMetricsF>
#include <QDebug>

TextBubble::TextBubble(ChatRole role, const QString &text, QWidget *parent)
    : BubbleFrame(role, parent)
{
    // 为什么用只读 QTextEdit 而不是 QLabel: 以后要支持富文本/粘贴图片, QTextEdit 更合适
    m_pTextEdit = new QTextEdit();
    m_pTextEdit->setReadOnly(true);
    m_pTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pTextEdit->installEventFilter(this);           // 高度要靠事件过滤器在绘制时调

    QFont font("Microsoft YaHei");
    font.setPointSize(12);
    m_pTextEdit->setFont(font);

    setPlainText(text);
    setWidget(m_pTextEdit); // 把文本编辑框塞进气泡
    initStyleSheet();
}

// 文本换行后的"实际高度"只有绘制时才知道, 所以在这里调
bool TextBubble::eventFilter(QObject *o, QEvent *e)
{
    if (m_pTextEdit == o && e->type() == QEvent::Paint) {
        adjustTextHeight();
    }
    return BubbleFrame::eventFilter(o, e);
}

void TextBubble::setPlainText(const QString &text)
{
    m_pTextEdit->setPlainText(text);

    //算出气泡的最大宽度: 取"最宽的那一段 "
    qreal doc_margin = m_pTextEdit->document()->documentMargin();   // 文档内边距(默认 4)
    int margin_left = this->layout()->contentsMargins().left();
    int margin_right = this->layout()->contentsMargins().right();

    QFontMetrics fm(m_pTextEdit->font());
    QTextDocument *doc = m_pTextEdit->document();
    int max_width = 0;
    for(QTextBlock it = doc->begin(); it != doc->end(); it = it.next()) {
        int txtw = int(fm.horizontalAdvance(it.text()));
        max_width = max_width < txtw ? txtw : max_width;
    }

    // 最大宽度只需要设一次
    setMaximumWidth(max_width + int(doc_margin) * 2 + (margin_left + margin_right));
}

void TextBubble::adjustTextHeight()
{
    qreal doc_margin = m_pTextEdit->document()->documentMargin();
    QTextDocument *doc = m_pTextEdit->document();
    qreal text_height = 0;
    for (QTextBlock it = doc->begin(); it != doc->end(); it = it.next()) {
        QTextLayout *pLayout = it.layout();
        QRectF text_rect = pLayout->boundingRect();
        text_height += text_rect.height();
    }
    int vMargin = this->layout()->contentsMargins().top();
    const int new_height = int(text_height + doc_margin * 2 + vMargin * 2);
    if (new_height != height()) {
        setFixedHeight(new_height);
    }
}

void TextBubble::initStyleSheet()
{
    // 让 QTextEdit 透明无边框, 下面 BubbleFrame 画的绿色/白色气泡才露得出来
    m_pTextEdit->setStyleSheet("QTextEdit{background:transparent;border:none}");
}
