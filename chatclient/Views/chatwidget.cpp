#include "chatwidget.h"
#include "ui_chatwidget.h"
#include "Views/senderwidget.h"
#include "Views/receivewidget.h"
#include "chatlistwidget.h"
#include <QDebug>
#include <QFileDialog>
#include "Controls/kernel.h"

ChatWidget::ChatWidget(QWidget *parent, int id) :
    QWidget(parent),
    ui(new Ui::ChatWidget),
    m_id(id)
{
    ui->setupUi(this);

    m_recordDialog = new ChatRecordDialog;
    connect(this, SIGNAL(SIG_ShowHistoryMsg(Friend*)), m_recordDialog, SLOT(slot_showHistroyMsg(Friend*)));
    // 设置滚动条显示最底端
    ui->listWidget->scrollToBottom();
}

ChatWidget::~ChatWidget()
{
    delete ui;
}

void ChatWidget::setData(Friend *data)
{
    ui->listWidget->clear(); // 清空原来的聊天内容
    m_recordDialog->setWindowTitle(data->getName());
    ui->nameLbl->setText(data->getName());
    auto messages = data->getMessages();
    for (int i = 0; i < messages.count(); ++i) {
        auto message = messages[i];
        // qDebug() << message.getContent();
        auto item = new QListWidgetItem;
        ui->listWidget->addItem(item);
        if (message.getMsgType() == MsgType::Sender) {
            auto widget = new SenderWidget(nullptr, data->getImage());
            ui->listWidget->setItemWidget(item, widget);
            connect(widget, &SenderWidget::sizeChanged, [=](QRect rect) mutable {
                item->setSizeHint(QSize(width() * 3 / 5, rect.height() + 20));
            });
            widget->setMessage(message.getContent());
        } else if (message.getMsgType() == MsgType::Receive) {
            auto widget = new ReceiveWidget(nullptr, Kernel::GetAvatarUrl());
            ui->listWidget->setItemWidget(item, widget);
            connect(widget, &ReceiveWidget::sizeChanged, [=](QRect rect) mutable {
                item->setSizeHint(QSize(width() * 3 / 5, rect.height() + 20));
            });
            widget->setMessage(message.getContent());
        }
        ui->listWidget->scrollToBottom();
    }
}

void ChatWidget::on_sendBtn_clicked() // 发送信息按钮
{
    // qDebug() << __func__;
    QString content = ui->sendEdit->toPlainText();  // 不带格式纯文本
    if (content.isEmpty()) return;
    // 将该信息注册到Friend里面
    ChatListItem* Chatitem = ChatListWidget::GetItem();
    Message message(content, QTime::currentTime().toString("hh:mm:ss"), Receive);
    Chatitem->updateFriend(message);
    // 将该信息发送出去
    emit SIG_SendChatMsg(Chatitem->getItem()->getId(), content);
    ui->sendEdit->clear();
    setData(Chatitem->getItem());
}


void ChatWidget::on_chatRecordBtn_clicked() // 聊天记录按钮点击事件
{
    // qDebug() << __func__;
    // 发送聊天记录拉取请求
    Friend* data = ChatListWidget::GetItem()->getItem();
    emit SIG_ShowHistoryMsg(data);

    // m_recordDialog->showNormal();
}


void ChatWidget::on_sendFileBtn_clicked() // 发送文件按钮点击事件
{
    qDebug() << __func__;
    QString filename = QFileDialog::getOpenFileName(this, "打开文件");
    if (!filename.isEmpty()) { // 用户选中了文件
        QFile* selectedFile = new QFile(filename);
        if (!selectedFile->open(QFile::ReadOnly)) {
            qDebug() << "打开用户想要传输的文件失败";
            return;
        }
        uint64_t filesize = selectedFile->size();
        ChatListItem* Chatitem = ChatListWidget::GetItem();

        emit SIG_SendFile(Chatitem->getItem()->getId(), filename, filesize);
    }
}

