#include "newfrienditem.h"
#include "ui_newfrienditem.h"

NewFriendItem::NewFriendItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NewFriendItem)
{
    ui->setupUi(this);
}

NewFriendItem::~NewFriendItem()
{
    delete ui;
}

void NewFriendItem::setInfo()
{
    ui->label_name->setText(m_friendName);
    ui->label_Introduction->setText("我是"+m_friendName);
    ui->label_icon->setPixmap(m_friendIcon);
}

void NewFriendItem::on_acceptBtn_clicked() // 接受好友申请同意按钮点击事件
{
    // 发送同意回复
    emit SIG_FriendRequestAccepted(m_friendId);
}

