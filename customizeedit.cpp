#include "customizeedit.h"

CustomizeEdit::CustomizeEdit(QWidget *parent)
    : QLineEdit(parent)
    , _max_len(0)          // 0 表示"不限长"
{
    // 每次文字变化都检查一次长度
    connect(this, &QLineEdit::textChanged, this, &CustomizeEdit::limitTextLength);
}

void CustomizeEdit::SetMaxLength(int maxLen)
{
    _max_len = maxLen;
}
