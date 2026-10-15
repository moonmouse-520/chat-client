#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "chatdialog.h"
#include "tcpmgr.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _login_dlg = new LoginDialog(this);
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_login_dlg);
    //创建和注册消息连接
    connect(_login_dlg, &LoginDialog::swichRegister, this, &MainWindow::SlotSwitchReg);

    connect(_login_dlg, &LoginDialog::switchReset,this,&MainWindow::SlotSwitchReset);

    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_switch_chatdlg, this, &MainWindow::SlotSwichChat);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SlotSwitchReg()
{
    if(_reg_dlg == nullptr) {
        _reg_dlg = new RegisterDialog(this);
        _reg_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
        _reg_dlg->hide();
        connect(_reg_dlg, &RegisterDialog::sigSwitchLogin, this, &MainWindow::SlotSwitchLogin);

    }
    //连接登录界面返回登录信号
    setCentralWidget(_reg_dlg);
    if (_login_dlg) {
        _login_dlg->hide();
    }
    _reg_dlg->show();
}

void MainWindow::SlotSwitchLogin()
{
    //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
    _login_dlg = new LoginDialog(this);
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_login_dlg);
    if (_reg_dlg) {
        _reg_dlg->hide();
    }
    _login_dlg->show();
    //连接登录界面注册信号
    connect(_login_dlg, &LoginDialog::swichRegister,this,&MainWindow::SlotSwitchReg);
    connect(_login_dlg, &LoginDialog::switchReset, this, &MainWindow::SlotSwitchReset);
}

void MainWindow::SlotSwitchReset()
{
    if(_reset_dlg == nullptr) {
        _reset_dlg = new ResetDialog(this);
        _reset_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
        _reset_dlg->hide();
        connect(_reset_dlg, &ResetDialog::switchLogin,this,&MainWindow::SlotSwitchLogin2);

    }
    setCentralWidget(_reset_dlg);
    if (_login_dlg) {
        _login_dlg->hide();
    }
    _reset_dlg->show();
}

void MainWindow::SlotSwitchLogin2()
{
    _login_dlg = new LoginDialog(this);
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_login_dlg);
    if (_reset_dlg) {
        _reset_dlg->hide();
    }
    _login_dlg->show();

    connect(_login_dlg, &LoginDialog::switchReset, this, &MainWindow::SlotSwitchReset);
    connect(_login_dlg, &LoginDialog::swichRegister, this, &MainWindow::SlotSwitchReg);
}

void MainWindow::SlotSwichChat()
{
    _chat_dlg = new ChatDialog(this);

    _chat_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_contacts_loaded,
            _chat_dlg, &ChatDialog::slot_contacts_loaded);

    if (_login_dlg) {
        _login_dlg->hide();
    }
    setCentralWidget(_chat_dlg);
    _chat_dlg->show();

    this->setMinimumSize(QSize(1050,900));
    this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
}

