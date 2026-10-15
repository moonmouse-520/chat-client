#ifndef SEARCHLIST_H
#define SEARCHLIST_H

#include <QListWidget>
#include <QEvent>
#include <QScrollBar>
#include <QDebug>
#include <QWheelEvent>
#include <memory>
#include "userdata.h"
class LoadingDlg;
class FindSuccessDlg;

class SearchList : public QListWidget
{
    Q_OBJECT
public:
    explicit SearchList(QWidget *parent = nullptr);
    void SetSearchEdit(QWidget* edit);//ChatDialog 把搜索框交给它
    void CloseFindDlg();// 关掉当前弹出的查找框
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;   // 鼠标进出/滚轮
private:
    void addTipItem();  // 加"灰色占位条 + 查找 uid/name 那一行"
    void waitPending(bool pending = true);      // 显示/隐藏"搜索中"遮罩
    QWidget* _search_edit = nullptr;
    bool _send_pending = false;     // 防重复发送
    LoadingDlg* _loading_dlg = nullptr;
    //  ★_find_dlg 可能是 FindSuccessDlg 或 FindFailDlg
    QDialog* _find_dlg = nullptr;        // 可能装 FindSuccessDlg 或 FindFailDlg
private slots:
    void slot_item_clicked(QListWidgetItem* item);
    void slot_user_search(std::shared_ptr<SearchInfo> si);
};



#endif // SEARCHLIST_H
