#include "chatitembase.h"

ChatItemBase::ChatItemBase(ChatRole role, QWidget *parent)
    : QWidget{parent}, m_role{role}
{
    // ---- 名字 ----
    m_pNameLabel = new QLabel();
    m_pNameLabel->setObjectName("chat_user_name");
    QFont font("Microsoft YaHei");
    font.setPointSize(9);
    m_pNameLabel->setFont(font);
    m_pNameLabel->setFixedHeight(20);

    // ---- 头像 ----
    m_pIconLabel = new QLabel();
    m_pIconLabel->setScaledContents(true);
    m_pIconLabel->setFixedSize(42, 42);

    // ---- 一个空的"气泡占位" ----
    m_pBubble = new QWidget();

    // ---- 网格布局: 2 行 3 列 ----
    QGridLayout *pGLayout = new QGridLayout();
    pGLayout->setVerticalSpacing(3);
    pGLayout->setHorizontalSpacing(3);
    pGLayout->setContentsMargins(3, 3, 3, 3);
    // 弹簧: 把内容往一侧挤
    QSpacerItem *pSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    if (m_role == ChatRole::Self) {
        // 自己: 名字右上角、头像最右、气泡在名字下面, 弹簧占最左列
        m_pNameLabel->setContentsMargins(0, 0, 8, 0);
        m_pNameLabel->setAlignment(Qt::AlignRight);
        pGLayout->addWidget(m_pNameLabel, 0, 1, 1, 1);
        pGLayout->addWidget(m_pIconLabel, 0, 2, 2, 1, Qt::AlignTop);   // 跨 2 行, 顶部对齐
        pGLayout->addItem(pSpacer,          1, 0, 1, 1);
        pGLayout->addWidget(m_pBubble,      1, 1, 1, 1);
        pGLayout->setColumnStretch(0, 2);      // 拉伸比: 左边空得多, 内容自然靠右
        pGLayout->setColumnStretch(1, 3);
    } else {
        // 别人: 头像最左, 名字和气泡在头像右边
        m_pNameLabel->setContentsMargins(8, 0, 0, 0);
        m_pNameLabel->setAlignment(Qt::AlignLeft);
        pGLayout->addWidget(m_pIconLabel, 0, 0, 2, 1, Qt::AlignTop);
        pGLayout->addWidget(m_pNameLabel, 0, 1, 1, 1);
        pGLayout->addWidget(m_pBubble,    1, 1, 1, 1);
        pGLayout->addItem(pSpacer,        2, 2, 1, 1);
        pGLayout->setColumnStretch(1, 3);
        pGLayout->setColumnStretch(2, 2);      // 右边空得多, 内容靠左
    }
    this->setLayout(pGLayout);
}

void ChatItemBase::setUserName(const QString &name)
{
    m_pNameLabel->setText(name);
}

void ChatItemBase::setUserIcon(const QPixmap &icon)
{
    m_pIconLabel->setPixmap(icon);
}

void ChatItemBase::setWidget(QWidget *w)
{
    QGridLayout *pGLayout = qobject_cast<QGridLayout *>(this->layout());
    auto old = pGLayout->replaceWidget(m_pBubble, w);   // 同位置替换
    delete m_pBubble;                        // 旧的占位删掉, 别泄漏
    delete old;
    m_pBubble = w;
}
