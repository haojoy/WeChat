#include "contactwidget.h"
#include "ui_contactwidget.h"
#include <QDebug>

ContactWidget::ContactWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ContactWidget)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(1);
    ui->listWidget_FriendReq->clear(); // 清空原来的列表
//    for (int i = 0; i < 5; i ++) {
//        auto item = new QListWidgetItem;
//        ui->listWidget_FriendReq->addItem(item);
//        auto widget = new NewFriendItem;
//        ui->listWidget_FriendReq->setItemWidget(item, widget);
//        item->setSizeHint(QSize(600, 100));
//    }
}

ContactWidget::~ContactWidget()
{
    delete ui;
}

void ContactWidget::setItem(int id)
{
    if (m_mapNewFriendItem.find(id) == m_mapNewFriendItem.end()) {
        auto item = new NewFriendItem;
        connect(item, SIGNAL(SIG_FriendRequestAccepted(int)), this, SLOT(slot_friendRequestAccepted(int)));
        m_mapNewFriendItem[id] = item;
    }
}

NewFriendItem *ContactWidget::getItem(int id)
{
    if (m_mapNewFriendItem.find(id) != m_mapNewFriendItem.end()) {
        return m_mapNewFriendItem[id];
    }
    return nullptr;
}

void ContactWidget::addNewFriend(int id)
{
    if (m_mapNewFriendItem.find(id) == m_mapNewFriendItem.end()) return;
    qDebug() << __func__;
    auto item = new QListWidgetItem;
    ui->listWidget_FriendReq->addItem(item);
    auto widget = m_mapNewFriendItem[id];
    widget->setInfo();
    ui->listWidget_FriendReq->setItemWidget(item, widget);
    item->setSizeHint(QSize(600, 100));
}

void ContactWidget::slot_friendRequestAccepted(int id)
{
    // 隐藏该widget
    m_mapNewFriendItem[id]->hide();
    m_mapNewFriendItem.erase(id);
    emit SIG_FriendRequestAccept(id);
}
