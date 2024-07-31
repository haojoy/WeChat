#include "chatrecorddialog.h"
#include "ui_chatrecorddialog.h"
#include <QListWidgetItem>
#include "Controls/kernel.h"
#include <QDebug>

ChatRecordDialog::ChatRecordDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChatRecordDialog)
{
    ui->setupUi(this);
}

ChatRecordDialog::~ChatRecordDialog()
{
    delete ui;
}

void ChatRecordDialog::slot_showHistroyMsg(Friend* data) {
    // 1. 从本地获取
    qDebug() << __func__;
    ui->chatListWidget->clear();
    auto messages = data->getMessages();
    if (!messages.isEmpty()) {
        for (int i = 0; i < messages.count(); ++i) {
            auto message = messages[i];
            qDebug() << __func__ << message.getContent();
            auto item = new QListWidgetItem(ui->chatListWidget);
            item->setSizeHint(QSize(551, 83));
            ui->chatListWidget->addItem(item);
            if (message.getMsgType() == MsgType::Sender) { // 朋友的消息
                auto widget = new HistoryItem(nullptr, data->getImage(), data->getName());
                ui->chatListWidget->setItemWidget(item, widget);
                widget->setMessage(message.getContent());
                widget->setTime(message.getTime());
            } else if (message.getMsgType() == MsgType::Receive) { // 来自自己的消息
                auto widget = new HistoryItem(nullptr, Kernel::GetAvatarUrl(), Kernel::GetName());
                ui->chatListWidget->setItemWidget(item, widget);
                widget->setMessage(message.getContent());
                widget->setTime(message.getTime());
            }
        }
        ui->chatListWidget->scrollToBottom();
    }
    this->showNormal();
    // 2. 从服务器获取
}
