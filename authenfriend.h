#ifndef AUTHENFRIEND_H
#define AUTHENFRIEND_H

#include <QDialog>
#include <memory>
#include "userdata.h"
namespace Ui {
class AuthenFriend;
}

class AuthenFriend : public QDialog
{
    Q_OBJECT

public:
    explicit AuthenFriend(QWidget *parent = nullptr);
    ~AuthenFriend();

    void SetApplyInfo(std::shared_ptr<ApplyInfo> info); // 把"谁申请我"填进来

private slots:
    void on_agree_btn_clicked();
    void on_reject_btn_clicked();

private:
    Ui::AuthenFriend *ui;
    std::shared_ptr<ApplyInfo> _info;
};

#endif // AUTHENFRIEND_H
