#ifndef APPLYFRIENDPAGE_H
#define APPLYFRIENDPAGE_H

#include <QWidget>
#include <memory>
#include <unordered_map>
#include "userdata.h"
#include "applyfrienditem.h"

namespace Ui {
class ApplyFriendPage;
}

class ApplyFriendPage : public QWidget
{
    Q_OBJECT

public:
    explicit ApplyFriendPage(QWidget *parent = nullptr);
    ~ApplyFriendPage();

    void AddNewApply(std::shared_ptr<ApplyInfo>);
protected:
    void paintEvent(QPaintEvent* event) override;   //让qss背景生效
private:
    std::unordered_map<int, ApplyFriendItem*> _apply_items;  // uid -> 那一行(去重/改状态用)
public slots:
    void slot_auth_rsp(const QJsonObject&);
signals:
    void sig_new_friend(const QJsonObject& json);
private:    int _last_auth_uid = 0;   // 正在认证的那个 uid(失败时用来更新那一行)

    Ui::ApplyFriendPage *ui;
};

#endif // APPLYFRIENDPAGE_H
