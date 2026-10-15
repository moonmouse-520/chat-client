QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QT += core gui network

SOURCES += \
    adduseritem.cpp \
    applyfriend.cpp \
    applyfrienditem.cpp \
    applyfriendlist.cpp \
    applyfriendpage.cpp \
    authenfriend.cpp \
    bubbleframe.cpp \
    chatdialog.cpp \
    chatitembase.cpp \
    chatpage.cpp \
    chatuserlist.cpp \
    chatuserwid.cpp \
    chatview.cpp \
    clickedoncelabel.cpp \
    conuseritem.cpp \
    contactuserlist.cpp \
    friendinfopage.cpp \
    clickedbtn.cpp \
    clickedlabel.cpp \
    customizeedit.cpp \
    findfaildlg.cpp \
    findsuccessdlg.cpp \
    friendlabel.cpp \
    global.cpp \
    httpmgr.cpp \
    listitembase.cpp \
    loadingdlg.cpp \
    logindialog.cpp \
    main.cpp \
    mainwindow.cpp \
    registerdialog.cpp \
    resetdialog.cpp \
    searchlist.cpp \
    statewidget.cpp \
    tcpmgr.cpp \
    textbubble.cpp \
    timerbtn.cpp \
    userdata.cpp \
    usermgr.cpp

HEADERS += \
    adduseritem.h \
    applyfriend.h \
    applyfrienditem.h \
    applyfriendlist.h \
    applyfriendpage.h \
    authenfriend.h \
    bubbleframe.h \
    chatdialog.h \
    chatitembase.h \
    chatpage.h \
    chatuserlist.h \
    chatuserwid.h \
    chatview.h \
    clickedoncelabel.h \
    conuseritem.h \
    contactuserlist.h \
    friendinfopage.h \
    clickedbtn.h \
    clickedlabel.h \
    customizeedit.h \
    findfaildlg.h \
    findsuccessdlg.h \
    friendlabel.h \
    global.h \
    httpmgr.h \
    listitembase.h \
    loadingdlg.h \
    logindialog.h \
    mainwindow.h \
    registerdialog.h \
    resetdialog.h \
    searchlist.h \
    singleton.h \
    statewidget.h \
    tcpmgr.h \
    textbubble.h \
    timerbtn.h \
    userdata.h \
    usermgr.h

FORMS += \
    adduseritem.ui \
    applyfriend.ui \
    applyfrienditem.ui \
    applyfriendpage.ui \
    authenfriend.ui \
    chatdialog.ui \
    chatpage.ui \
    conuseritem.ui \
    friendinfopage.ui \
    chatuserwid.ui \
    findfaildlg.ui \
    findsuccessdlg.ui \
    friendlabel.ui \
    loadingdlg.ui \
    logindialog.ui \
    mainwindow.ui \
    registerdialog.ui \
    resetdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RC_ICONS = fhlz.ico
DESTDIR = ./bin

RESOURCES += \
    rc.qrc

DISTFILES += \
    config.ini

# 将工程目录下的 config.ini 拷贝到构建输出目录（debug/release 通用）
CONFIG += file_copies

copydata.files = $$PWD/config.ini      # 要拷贝的文件，可写多个
copydata.path  = $$OUT_PWD/$$DESTDIR   # 目标目录（构建输出目录）
COPIES += copydata
