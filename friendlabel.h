#ifndef FRIENDLABEL_H
#define FRIENDLABEL_H

#include <QFrame>
#include <QString>

namespace Ui { class FriendLabel; }

// ============================================================
class FriendLabel : public QFrame
{
    Q_OBJECT
public:
    explicit FriendLabel(QWidget *parent = nullptr);
    ~FriendLabel();

    void SetText(QString text);     // 设置文字, 并算出自身宽高
    int Width();                    // 缓存下来的宽
    int Height();                   // 缓存下来的高
    QString Text();                 // 缓存下来的文字(也是"我是谁"的凭据)

private:
    Ui::FriendLabel *ui;
    QString _text;
    int _width;
    int _height;

public slots:
    void slot_close();              // 点 × 时被调用

signals:
    void sig_close(QString);        // 通知弹窗: 把我删掉
};

#endif // FRIENDLABEL_H
