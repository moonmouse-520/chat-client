#ifndef CUSTOMIZEEDIT_H
#define CUSTOMIZEEDIT_H

#include <QLineEdit>
#include <QDebug>

class CustomizeEdit : public QLineEdit
{
    Q_OBJECT
public:
    CustomizeEdit(QWidget *parent = nullptr);

    void SetMaxLength(int maxLen);

protected:
    void focusOutEvent(QFocusEvent *event) override
    {
        QLineEdit::focusOutEvent(event);
        emit sig_foucus_out();
    }

private:
    // 文字变化时按 UTF-8 字节截断
    void limitTextLength(QString text)
    {
        if (_max_len <= 0) {
            return;
        }
        if (text.toUtf8().size() <= _max_len) {
            return;
        }

        // 按"完整字符"逐个累加字节数, 到上限就停。
        int bytes = 0;
        QString out;
        for (const QChar& ch : text) {
            const int b = QString(ch).toUtf8().size();
            if (bytes + b > _max_len) {
                break;
            }
            bytes += b;
            out.append(ch);
        }
        if (out.size() != text.size()) {
            this->setText(out);
        }
    }

    int _max_len;

signals:
    void sig_foucus_out();
};

#endif // CUSTOMIZEEDIT_H
