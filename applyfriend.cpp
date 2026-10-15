#include "applyfriend.h"
#include "ui_applyfriend.h"
#include "clickedlabel.h"
#include "friendlabel.h"
#include <QScrollBar>
#include "usermgr.h"
#include "tcpmgr.h"
#include "global.h"
#include <QJsonObject>
#include <QJsonDocument>
ApplyFriend::ApplyFriend(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ApplyFriend),_label_point(2,6)
{
    ui->setupUi(this);
    // 隐藏对话框标题栏
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    this->setObjectName("ApplyFriend");
    this->setModal(true);
    ui->name_ed->setPlaceholderText(tr("恋恋风辰"));
    ui->lb_ed->setPlaceholderText("搜索、添加标签");
    ui->back_ed->setPlaceholderText("燃烧的胸毛");

	ui->lb_ed->SetMaxLength(21);
	ui->lb_ed->move(2, 2);
	ui->lb_ed->setFixedHeight(20);
	ui->lb_ed->setMaxLength(10);
	ui->input_tip_wid->hide();

    _tip_cur_point = QPoint(5, 5);

	_tip_data = { "同学","家人","菜鸟教程","C++ Primer","Rust 程序设计",
							 "父与子学Python","nodejs开发指南","go 语言开发指南",
								"游戏伙伴","金融投资","微信读书","拼多多拼友" };

    connect(ui->more_lb, &ClickedOnceLabel::clicked, this, &ApplyFriend::ShowMoreLabel);
    InitTipLbs();
    //链接输入标签回车事件
    connect(ui->lb_ed, &CustomizeEdit::returnPressed, this, &ApplyFriend::SlotLabelEnter);
    connect(ui->lb_ed, &CustomizeEdit::textChanged, this, &ApplyFriend::SlotLabelTextChange);
    connect(ui->lb_ed, &CustomizeEdit::editingFinished, this, &ApplyFriend::SlotLabelEditFinished);
    connect(ui->tip_lb, &ClickedOnceLabel::clicked, this, &ApplyFriend::SlotAddFirendLabelByClickTip);

    ui->scrollArea->horizontalScrollBar()->setHidden(true);
    ui->scrollArea->verticalScrollBar()->setHidden(true);
    ui->scrollArea->installEventFilter(this);
    ui->sure_btn->SetState("normal","hover","press");
    ui->cancel_btn->SetState("normal","hover","press");
    //连接确认和取消按钮的槽函数
    connect(ui->cancel_btn, &QPushButton::clicked, this, &ApplyFriend::SlotApplyCancel);
    connect(ui->sure_btn, &QPushButton::clicked, this, &ApplyFriend::SlotApplySure);
}

ApplyFriend::~ApplyFriend()
{
    qDebug()<< "ApplyFriend destruct";
    delete ui;
}

void ApplyFriend::InitTipLbs()
{
    int lines = 1;
    for(int i = 0; i < (int)_tip_data.size(); i++){

		auto* lb = new ClickedLabel(ui->lb_list);
		lb->SetState("normal", "hover", "pressed", "selected_normal",
			"selected_hover", "selected_pressed");
		lb->setObjectName("tipslb");
		lb->setText(_tip_data[i]);
		connect(lb, &ClickedLabel::clicked, this, &ApplyFriend::SlotChangeFriendLabelByTip);

		QFontMetrics fontMetrics(lb->font()); // 获取QLabel控件的字体信息
		int textWidth = fontMetrics.width(lb->text()); // 获取文本的宽度
		int textHeight = fontMetrics.height(); // 获取文本的高度

		if (_tip_cur_point.x() + textWidth + tip_offset > ui->lb_list->width()) {
            lines++;
            if (lines > 2) {
                delete lb;
                return;
            }
			
            _tip_cur_point.setX(tip_offset);
            _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);

		}

       auto next_point = _tip_cur_point;

       AddTipLbs(lb, _tip_cur_point,next_point, textWidth, textHeight);

       _tip_cur_point = next_point;
    }

}


// ------------------------------------------------------------
// 把一个推荐标签"摆到游标处", 并把游标推到它右边
//   next_point 是引用: 调用方拿着它当下一个标签的起点
//   +15 是标签之间的间距
// ------------------------------------------------------------
void ApplyFriend::AddTipLbs(ClickedLabel* lb, QPoint cur_point, QPoint& next_point, int text_width, int text_height)
{
	lb->move(cur_point);
	lb->show();
	_add_labels.insert(lb->text(), lb);
	_add_label_keys.push_back(lb->text());
	next_point.setX(lb->pos().x() + text_width + 15);
    next_point.setY(lb->pos().y());
}

bool ApplyFriend::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->scrollArea && event->type() == QEvent::Enter)
    {
        ui->scrollArea->verticalScrollBar()->setHidden(false);
    }
    else if (obj == ui->scrollArea && event->type() == QEvent::Leave)
    {
        ui->scrollArea->verticalScrollBar()->setHidden(true);
    }
    return QObject::eventFilter(obj, event);
}

// ------------------------------------------------------------
// 打开弹窗时把"搜到的这个人"填进去(FindSuccessDlg 调)
//   申请附言默认写成"您好,我是<我的名字>", 备注名默认用对方的名字 ——
//   都只是预填, 用户可以改
// ------------------------------------------------------------
void ApplyFriend::SetSearchInfo(std::shared_ptr<SearchInfo> si)
{
    _si = si;
    auto applyname = UserMgr::GetInstance()->GetName();
    auto bakname = si->_name;
    QString str = QString("您好,我是%1").arg(applyname);
    ui->name_ed->setText(str);
    ui->back_ed->setText(bakname);
}

// ------------------------------------------------------------
// 点右箭头: 展开全部推荐标签
//   more_lb_wid 先 hide, lb_list 宽度从 287 撑到 325,
//   然后把已经建过的标签重排一遍, 再把 _tip_data 里还没建的补建出来,
//   最后按内容高度调 lb_list 的固定高、并把多出来的高度加给 scrollcontent
// ------------------------------------------------------------
void ApplyFriend::ShowMoreLabel()
{
    qDebug()<< "receive more label clicked";
    ui->more_lb_wid->hide();

    ui->lb_list->setFixedWidth(325);
    _tip_cur_point = QPoint(5, 5);
    auto next_point = _tip_cur_point;
    int textWidth;
    int textHeight;
    //重拍现有的label
    for(auto & added_key : _add_label_keys){
        auto added_lb = _add_labels[added_key];

        QFontMetrics fontMetrics(added_lb->font()); // 获取QLabel控件的字体信息
        textWidth = fontMetrics.width(added_lb->text()); // 获取文本的宽度
        textHeight = fontMetrics.height(); // 获取文本的高度

        if(_tip_cur_point.x() +textWidth + tip_offset > ui->lb_list->width()){
            _tip_cur_point.setX(tip_offset);
            _tip_cur_point.setY(_tip_cur_point.y()+textHeight+15);
        }
        added_lb->move(_tip_cur_point);

        next_point.setX(added_lb->pos().x() + textWidth + 15);
        next_point.setY(_tip_cur_point.y());

        _tip_cur_point = next_point;

    }

    //添加未添加的
    for(int i = 0; i < _tip_data.size(); i++){
        auto iter = _add_labels.find(_tip_data[i]);
        if(iter != _add_labels.end()){
            continue;
        }

		auto* lb = new ClickedLabel(ui->lb_list);
		lb->SetState("normal", "hover", "pressed", "selected_normal",
			"selected_hover", "selected_pressed");
		lb->setObjectName("tipslb");
		lb->setText(_tip_data[i]);
		connect(lb, &ClickedLabel::clicked, this, &ApplyFriend::SlotChangeFriendLabelByTip);

		QFontMetrics fontMetrics(lb->font()); // 获取QLabel控件的字体信息
		int textWidth = fontMetrics.width(lb->text()); // 获取文本的宽度
		int textHeight = fontMetrics.height(); // 获取文本的高度

		if (_tip_cur_point.x() + textWidth + tip_offset > ui->lb_list->width()) {

			_tip_cur_point.setX(tip_offset);
			_tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);

		}

		 next_point = _tip_cur_point;

		AddTipLbs(lb, _tip_cur_point, next_point, textWidth, textHeight);

		_tip_cur_point = next_point;

    }

   int diff_height = next_point.y() + textHeight + tip_offset - ui->lb_list->height();
   ui->lb_list->setFixedHeight(next_point.y() + textHeight + tip_offset);

    //qDebug()<<"after resize ui->lb_list size is " <<  ui->lb_list->size();
    ui->scrollcontent->setFixedHeight(ui->scrollcontent->height()+diff_height);
}

// ------------------------------------------------------------
// 按当前 _friend_labels / _friend_label_keys 重排"已选标签区"
//   删除一个标签之后要调它 —— 否则右边会留个空洞。
//   它是遍历 QMap 的(按 key 字典序), 删完标签顺序会变
// ------------------------------------------------------------
void ApplyFriend::resetLabels()
{
    auto max_width = ui->gridWidget->width();
    auto label_height = 0;
    for(auto iter = _friend_labels.begin(); iter != _friend_labels.end(); iter++){
        if( _label_point.x() + iter.value()->width() > max_width) {
            _label_point.setY(_label_point.y()+iter.value()->height()+6);
            _label_point.setX(2);
        }

        iter.value()->move(_label_point);
        iter.value()->show();

        _label_point.setX(_label_point.x()+iter.value()->width()+2);
        _label_point.setY(_label_point.y());
        label_height = iter.value()->height();
    }

    if(_friend_labels.isEmpty()){
         ui->lb_ed->move(_label_point);
         return;
    }

    if(_label_point.x() + MIN_APPLY_LABEL_ED_LEN > ui->gridWidget->width()){
        ui->lb_ed->move(2,_label_point.y()+label_height+6);
    }else{
         ui->lb_ed->move(_label_point);
    }
}

void ApplyFriend::addLabel(QString name)
{
    if (_friend_labels.find(name) != _friend_labels.end()) {
        ui->lb_ed->clear();
        return;
    }

	auto tmplabel = new FriendLabel(ui->gridWidget);
	tmplabel->SetText(name);
	tmplabel->setObjectName("FriendLabel");

	auto max_width = ui->gridWidget->width();
	if (_label_point.x() + tmplabel->width() > max_width) {
		_label_point.setY(_label_point.y() + tmplabel->height() + 6);
		_label_point.setX(2);
	}
	else {

	}


	tmplabel->move(_label_point);
	tmplabel->show();
	_friend_labels[tmplabel->Text()] = tmplabel;
	_friend_label_keys.push_back(tmplabel->Text());

	connect(tmplabel, &FriendLabel::sig_close, this, &ApplyFriend::SlotRemoveFriendLabel);

	_label_point.setX(_label_point.x() + tmplabel->width() + 2);

	if (_label_point.x() + MIN_APPLY_LABEL_ED_LEN > ui->gridWidget->width()) {
		ui->lb_ed->move(2, _label_point.y() + tmplabel->height() + 2);
	}
	else {
		ui->lb_ed->move(_label_point);
	}

	ui->lb_ed->clear();

	if (ui->gridWidget->height() < _label_point.y() + tmplabel->height() + 2) {
		ui->gridWidget->setFixedHeight(_label_point.y() + tmplabel->height() * 2 + 2);
	}
}

// ------------------------------------------------------------
// 在输入框里按回车 -> 把刚输入的文字变成一个已选标签
// -----------------------------------------------------------
void ApplyFriend::SlotLabelEnter()
{
    if(ui->lb_ed->text().isEmpty()){
        return;
    }

    auto text = ui->lb_ed->text();

    addLabel(ui->lb_ed->text());

    ui->input_tip_wid->hide();

    auto find_it = std::find(_tip_data.begin(), _tip_data.end(), text);
    //找到了就只需设置状态为选中即可
    if (find_it == _tip_data.end()) {
        _tip_data.push_back(text);
    }

    //判断标签展示栏是否有该标签
    auto find_add = _add_labels.find(text);
    if (find_add != _add_labels.end()) {
        find_add.value()->SetCurState(ClickLbState::Selected);
        return;
    }

    //标签展示栏也增加一个标签, 并设置绿色选中
    auto* lb = new ClickedLabel(ui->lb_list);
    lb->SetState("normal", "hover", "pressed", "selected_normal",
        "selected_hover", "selected_pressed");
    lb->setObjectName("tipslb");
    lb->setText(text);
    connect(lb, &ClickedLabel::clicked, this, &ApplyFriend::SlotChangeFriendLabelByTip);
    qDebug() << "ui->lb_list->width() is " << ui->lb_list->width();
    qDebug() << "_tip_cur_point.x() is " << _tip_cur_point.x();

    QFontMetrics fontMetrics(lb->font()); // 获取QLabel控件的字体信息
    int textWidth = fontMetrics.width(lb->text()); // 获取文本的宽度
    int textHeight = fontMetrics.height(); // 获取文本的高度
    qDebug() << "textWidth is " << textWidth;

    if (_tip_cur_point.x() + textWidth + tip_offset + 3 > ui->lb_list->width()) {

        _tip_cur_point.setX(5);
        _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);

    }

    auto next_point = _tip_cur_point;

    AddTipLbs(lb, _tip_cur_point, next_point, textWidth, textHeight);
    _tip_cur_point = next_point;

    int diff_height = next_point.y() + textHeight + tip_offset - ui->lb_list->height();
    ui->lb_list->setFixedHeight(next_point.y() + textHeight + tip_offset);

    lb->SetCurState(ClickLbState::Selected);

    ui->scrollcontent->setFixedHeight(ui->scrollcontent->height() + diff_height);
}

// ------------------------------------------------------------
// 删掉一个已选标签
// ------------------------------------------------------------
void ApplyFriend::SlotRemoveFriendLabel(QString name)
{
    qDebug() << "receive close signal";

    _label_point.setX(2);
    _label_point.setY(6);

   auto find_iter = _friend_labels.find(name);

   if(find_iter == _friend_labels.end()){
       return;
   }

   auto find_key = _friend_label_keys.end();
   for(auto iter = _friend_label_keys.begin(); iter != _friend_label_keys.end();
       iter++){
       if(*iter == name){
           find_key = iter;
           break;
       }
   }

   if(find_key != _friend_label_keys.end()){
      _friend_label_keys.erase(find_key);
   }


   delete find_iter.value();

   _friend_labels.erase(find_iter);

   resetLabels();

   auto find_add = _add_labels.find(name);
   if(find_add == _add_labels.end()){
        return;
   }

   find_add.value()->ResetNormalState();
}

//点击标已有签添加或删除新联系人的标签
// ------------------------------------------------------------
// 点推荐标签栏里的某个标签
//   信号带出来的 state 是"点击之后"的状态:
//     Selected -> 加进已选区
//     Normal   -> 从已选区删掉(等于用户又点了一下取消)
// ------------------------------------------------------------
void ApplyFriend::SlotChangeFriendLabelByTip(QString lbtext, ClickLbState state)
{
    auto find_iter = _add_labels.find(lbtext);
    if(find_iter == _add_labels.end()){
        return;
    }

    if(state == ClickLbState::Selected){
        //编写添加逻辑
        addLabel(lbtext);
        return;
    }

    if(state == ClickLbState::Normal){
        //编写删除逻辑
        SlotRemoveFriendLabel(lbtext);
        return;
    }

}

// ------------------------------------------------------------
// 输入框内容变了 -> 显示/隐藏下面那条提示("添加标签 xxx")
//   文字是空的 -> 隐藏提示条;
//   文字已经在推荐列表里 -> 提示条就显示这个文字;
//   是新标签 -> 提示条显示 add_prefix + 文字(用户点它就能直接加)
// ------------------------------------------------------------
void ApplyFriend::SlotLabelTextChange(const QString& text)
{
    if (text.isEmpty()) {
        ui->tip_lb->setText("");
        ui->input_tip_wid->hide();
        return;
    }

    auto iter = std::find(_tip_data.begin(), _tip_data.end(), text);
    if (iter == _tip_data.end()) {
        auto new_text = add_prefix + text;
        ui->tip_lb->setText(new_text);
        ui->input_tip_wid->show();
        return;
    }
    ui->tip_lb->setText(text);
    ui->input_tip_wid->show();
}

// ------------------------------------------------------------
// 输入框失去焦点 / 结束编辑 -> 把提示条藏起来
// ------------------------------------------------------------
void ApplyFriend::SlotLabelEditFinished()
{
    ui->input_tip_wid->hide();
}

// ------------------------------------------------------------
// 点"添加标签 xxx"提示条
//    提示条上的文字是带前缀的("添加标签 老乡"), 所以要先 indexOf(add_prefix)
//     找到前缀位置, 再用 mid() 把它切掉, 剩下的才是真正的标签名。
//   后面就是把标签加进已选区 + 在推荐栏里点亮/新建(和回车那条路一样)
// ------------------------------------------------------------
void ApplyFriend::SlotAddFirendLabelByClickTip(QString text)
{
    int index = text.indexOf(add_prefix);
    if (index != -1) {
        text = text.mid(index + add_prefix.length());
    }
    addLabel(text);

    auto find_it = std::find(_tip_data.begin(), _tip_data.end(), text);
    //找到了就只需设置状态为选中即可
    if (find_it == _tip_data.end()) {
        _tip_data.push_back(text);
    }
   
    //判断标签展示栏是否有该标签
    auto find_add = _add_labels.find(text);
    if (find_add != _add_labels.end()) {
        find_add.value()->SetCurState(ClickLbState::Selected);
        return;
    }
     
    //标签展示栏也增加一个标签, 并设置绿色选中
	auto* lb = new ClickedLabel(ui->lb_list);
	lb->SetState("normal", "hover", "pressed", "selected_normal",
		"selected_hover", "selected_pressed");
	lb->setObjectName("tipslb");
	lb->setText(text);
	connect(lb, &ClickedLabel::clicked, this, &ApplyFriend::SlotChangeFriendLabelByTip);
    qDebug() << "ui->lb_list->width() is " << ui->lb_list->width();
    qDebug() << "_tip_cur_point.x() is " << _tip_cur_point.x();
   
	QFontMetrics fontMetrics(lb->font()); // 获取QLabel控件的字体信息
	int textWidth = fontMetrics.width(lb->text()); // 获取文本的宽度
	int textHeight = fontMetrics.height(); // 获取文本的高度
    qDebug() << "textWidth is " << textWidth;

	if (_tip_cur_point.x() + textWidth+ tip_offset+3 > ui->lb_list->width()) {

		_tip_cur_point.setX(5);
		_tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);

	}

	auto next_point = _tip_cur_point;

	 AddTipLbs(lb, _tip_cur_point, next_point, textWidth,textHeight);
	_tip_cur_point = next_point;

    int diff_height = next_point.y() + textHeight + tip_offset - ui->lb_list->height();
    ui->lb_list->setFixedHeight(next_point.y() + textHeight + tip_offset);

    lb->SetCurState(ClickLbState::Selected);

    ui->scrollcontent->setFixedHeight(ui->scrollcontent->height()+ diff_height );
}

// ============================================================
// 点"确认" -> 把这次申请发给服务端(1009)
// ============================================================
void ApplyFriend::SlotApplySure()
{
    if (_si == nullptr) {
        // 正常流程不会走到这, 加个判空防崩
        this->hide();
        deleteLater();
        return;
    }

    QJsonObject jsonObj;

    // desc: 申请附言("您好,我是xxx")
    QString apply_name = ui->name_ed->text();
    if (apply_name.isEmpty()) {
        apply_name = ui->name_ed->placeholderText();
    }
    jsonObj["desc"] = apply_name;

    // bakname: 我给他起的备注名
    //   服务端会存进 friend_apply.back_name; 等对方同意后, 写进
    //   friend(self_id=我, friend_id=他).back —— 也就是"我看他"时显示的名字
    QString bakname = ui->back_ed->text();
    if (bakname.isEmpty()) {
        bakname = ui->back_ed->placeholderText();
    }
    jsonObj["bakname"] = bakname;

    // 申请谁: 搜索出来的那个人
    jsonObj["applyuid"] = _si->_uid;

    // ★ 我们项目的 sig_send_data 第二个参数是 QString(教程版是 QByteArray)
    QString jsonStr = QString(QJsonDocument(jsonObj).toJson(QJsonDocument::Compact));
    emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_ADD_FRIEND_REQ, jsonStr);

    this->hide();
    deleteLater();
}

// ------------------------------------------------------------
// 点"取消": 什么都不发, 关掉自己
//    用 deleteLater() 而不是 delete —— 我们现在还在自己的槽函数里,
//     直接 delete 会在返回后访问已释放对象(闪退)
// ------------------------------------------------------------
void ApplyFriend::SlotApplyCancel()
{
    qDebug() << "Slot Apply Cancel";
    this->hide();
    deleteLater();
}

