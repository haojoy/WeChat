#include "userinfoshowdialog.h"
#include "ui_userinfoshowdialog.h"
#include <QDebug>

UserInfoShowDialog::UserInfoShowDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UserInfoShowDialog)
{
    ui->setupUi(this);
    // setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);

    // 设置窗口标志为工具窗口
    setWindowFlags(windowFlags() | Qt::Tool | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);

    // 隐藏任务栏图标
    setAttribute(Qt::WA_ShowWithoutActivating);

    // 修改显示位置

}

UserInfoShowDialog::~UserInfoShowDialog()
{
    delete ui;
}

void UserInfoShowDialog::setUserName(QString username) {
    ui->label_username->setText(username);
}

void UserInfoShowDialog::setUserAvatar(QByteArray& byteArray) {
    QPixmap pixmap;
    pixmap.loadFromData(byteArray);
    if (pixmap.isNull()) {
        qDebug() << "头像数据错误";
        return;
    }
    ui->iconBtn->setIcon(QIcon(pixmap));

}

void UserInfoShowDialog::setUserIcon(QPixmap& pixmap) {
    ui->iconBtn->setIcon(QIcon(pixmap));
}

void UserInfoShowDialog::on_addFriendBtn_clicked()
{
    // 发送添加好友声请
    QString friendName = ui->label_username->text();
    emit SIG_AddFriendRequest(friendName, m_userId);
    this->hide();
}

