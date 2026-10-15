#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointer>          // 修 bug: 会自动变空的指针
#include "logindialog.h"
#include <registerdialog.h>
#include "resetdialog.h"
#include "chatdialog.h"
/********************************************************************************
*
* @file        mainwindow.h
* @brief       主窗口
*
* @author      徐昊
* @date        2026/08/18
* @history
********************************************************************************/

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
public slots:
    void SlotSwitchReg();
    void SlotSwitchLogin();
    void SlotSwitchReset();
    void SlotSwitchLogin2();
    void SlotSwichChat();
private:
    Ui::MainWindow *ui;
    QPointer<LoginDialog>    _login_dlg    = nullptr;
    QPointer<RegisterDialog> _reg_dlg      = nullptr;
    QPointer<ResetDialog>    _reset_dlg    = nullptr;
    QPointer<ChatDialog>     _chat_dlg     = nullptr;
};
#endif // MAINWINDOW_H
