#include "loadingdlg.h"
#include "ui_loadingdlg.h"
#include <QLabel>
#include <QMovie>
#include <QPoint>
LoadingDlg::LoadingDlg(QWidget *parent, QString tip)
    : QDialog(parent)
    , ui(new Ui::LoadingDlg)
{
    ui->setupUi(this);
    //   FramelessWindowHint     去掉标题栏和边框
    //   WindowStaysOnTopHint    压在最上面, 不会被父窗口盖住
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint
                   | Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);     //设置背景透明

    // 盖住父窗口: 尺寸跟父窗口一样, 位置对齐父窗口左上角
    if(parent) {
        setFixedSize(parent->size());   // 固定住, 防被布局再改
        //  move() 要的是【屏幕坐标】, 而 parent->pos() 是【父控件的坐标】
        //   mapToGlobal(QPoint(0,0)) = 把"本控件内的(0,0)"换算成屏幕上的点
        move(parent->mapToGlobal(QPoint(0,0)));
    }

    // ----  播 gif ----
    QMovie *movie = new QMovie(":/res/loading.gif", QByteArray(), this);
     ui->loading_lb->setMovie(movie);
    movie->setScaledSize(ui->loading_lb->size());
    movie->start();

    ui->status_lb->setText(tip);
}

LoadingDlg::~LoadingDlg()
{
    delete ui;
}
