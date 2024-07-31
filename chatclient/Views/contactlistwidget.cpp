#include "contactlistwidget.h"
#include "ui_contactlistwidget.h"
#include <QDebug>
#include <QMouseEvent>
#include <QApplication>
#include <QListWidget>

ContactListWidget::ContactListWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ContactListWidget)
{
    ui->setupUi(this);
}

ContactListWidget::~ContactListWidget()
{
    delete ui;
}

void ContactListWidget::on_addBtn_clicked() // 转到添加好友界面按钮点击事件
{
    ui->contactStackedWidget->setCurrentIndex(1);
}


void ContactListWidget::on_cancelBtn_clicked() // 转到好友列表界面按钮点击事件
{
    ui->contactStackedWidget->setCurrentIndex(0);
}


void ContactListWidget::on_lineEdit_textChanged(const QString &arg1) // 添加好友输入框文本变化事件
{
    qDebug()<< __func__;
    // 应该在Widget中显示Button用于当作确认
    ui->searchStackedWidget->setCurrentIndex(0);
    ui->label_confirm->setText(arg1);
}

void ContactListWidget::modifySearchStackedWidgetIndex() {
    ui->searchStackedWidget->setCurrentIndex(1);
}

void ContactListWidget::on_addFriendBtn_clicked() // 添加好友按钮点击事件
{
    qDebug()<< __func__;
    // 获取QLineEdit的文本
    QString username = ui->lineEdit->text();
    // 判断文本是否合法
    if (username.isEmpty()) {
        ui->searchStackedWidget->setCurrentIndex(1);
        return;
    }
    // 向Kenel发送添加好友信号
    emit SIG_GetFriendInfo(username);
}

