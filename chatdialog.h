#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>
#include <QJsonObject>
#include "global.h"
#include "statewidget.h"
#include "userdata.h"
#include <QListWidgetItem>
#include <QMap>
#include <QSet>
#include "chatuserwid.h"
namespace Ui {
class ChatDialog;
}

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatDialog(QWidget *parent = nullptr);
    ~ChatDialog();

public slots:
    void slot_contacts_loaded(const QJsonObject& json);
    void slot_friend_apply(std::shared_ptr<ApplyInfo> info);
private slots:
    void slot_side_chat();          // 侧边栏: 聊天
    void slot_side_contact();       // 侧边栏: 联系人
    void slot_side_apply();         // 侧边栏: 申请(页面等教程后续集数)
    void slot_add_btn_clicked();    // "+" : 展开/收起搜索
    void slot_loading_chat_user();  // 列表滚到底
    void slot_text_changed(const QString& str);
    void slot_friend_auth(const QJsonObject& json);
    void slot_chat_user_clicked(QListWidgetItem* item);
    void slot_friend_info_page(std::shared_ptr<FriendInfo> friend_info);             // 点联系人行 -> 资料页
    void slot_jump_chat_item_from_infopage(std::shared_ptr<FriendInfo> friend_info); // 资料页点"发消息"
    void slot_recv_msg(const QJsonObject& json);
    void slot_new_friend(const QJsonObject& json);
    void slot_msg_sent(int uid, const QString& text, const QString& server_time = QString());
private:
    void initSelfInfo();                     // 左下角显示自己的头像
    void initSearchEdit();                   // 搜索框的两个 QAction + 限长
    void ShowSearch(bool bsearch = false);   // 切换"聊天列表 / 搜索列表"
    void addChatUserItem(const QString& name, const QString& head,
                         const QString& msg, int uid = 0,int thread_id = 0);   // 往列表里加一行
    void updateChatUserThreadId(int uid, int thread_id);
    void AddLBGroup(StateWidget *lb);   //把按钮登记进 _lb_list
    void ClearLabelState(StateWidget *lb); //清掉除我之外所有按钮的选中状态
    void handleGlobalMousePress(QMouseEvent *event);// 处理"点空白"

    Ui::ChatDialog *ui;
    ChatUIMode _mode;      // 当前模式(搜索/聊天/联系人)
    ChatUIMode _state;     // 上一次的"稳定"模式 —— 从搜索退出时要回到它
    bool _b_loading;       // 是否正在加载下一页
    QList<StateWidget*> _lb_list;// 侧边栏三个按钮
    QMap<int, ChatUserWid*> _user_items;
    QSet<int> _unread_uids;              // 哪些 uid 还没读(1026 重建列表后要还原红点)
    QMap<int, QString> _last_msg_text;   // uid -> 最后一条消息(1026 不带这个信息)
    QMap<int, QString> _last_msg_time;   // uid -> 那条消息的时间(1026 也不带, 重建时要贴回)
    void SetUnread(int uid, bool unread);
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // CHATDIALOG_H
