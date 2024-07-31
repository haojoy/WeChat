#include "chatlistitem.h"
#include "ui_chatlistitem.h"
#include <QEvent>

ChatListItem::ChatListItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatListItem)
{
    ui->setupUi(this);
    this->installEventFilter(this);
}

ChatListItem::~ChatListItem()
{
    delete ui;
}

void ChatListItem::setItem(Friend *data)
{
    if (data == nullptr) return;
    m_data = data;
    auto pixmap = QPixmap(data->getImage());
    ui->headLbl->setPixmap(pixmap);
    ui->nameLbl->setText(data->getName());
    ui->timeLbl->setText(data->getTime());
    if (data->getMessages().count() > 0) {     // 将最后一条未读消息设置到聊天列表项中
        setContent(data->getMessages().last().getContent());
    }
    ui->countLbl->setText(QString::number(data->getCount()));
    ui->widget_5->setVisible(data->getCount() != 0);
    ui->muteLbl->setVisible(data->getMute());
}

void ChatListItem::setContent(QString content)
{
    QFontMetrics metrics(font()); // 字体度量对象，用于测量文本的宽度。
    // 根据给定的最大宽度(150像素)对文本进行截断，并添加省略号（...）以表示被截断了。截断的位置是在文本的右侧
    auto new_content = metrics.elidedText(content, Qt::ElideRight, 150);
    ui->messLbl->setText(new_content);
}

void ChatListItem::updateContent(Message& messages) {
    // 往消息队列中添加消息
    m_data->addMessage(messages);
    m_data->addCount(1);
    ui->countLbl->setText(QString::number(m_data->getCount()));
    ui->widget_5->setVisible(m_data->getCount() != 0);
    m_data->setTime(messages.getTime());
    if (m_data->getMessages().count() > 0) {     // 将最后一条未读消息设置到聊天列表项中
        setContent(m_data->getMessages().last().getContent());
    }
    ui->timeLbl->setText(m_data->getTime());
}

void ChatListItem::updateFriend(Message& message) {
    m_data->addMessage(message);
}

void ChatListItem::Checked(bool checked)
{
    m_isChecked = checked;
}

void ChatListItem::Selected(bool selected)
{
    if (selected)
    {
        ui->widget->setStyleSheet("QWidget#widget{background: #c3c3c3;}");
    }
    else
    {
        ui->widget->setStyleSheet("QWidget#widget{background: #dfdfdf;}");
    }
}

//Friend *ChatListItem::item()
//{
//    return this->data;
//}

void ChatListItem::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    ui->countLbl->setText("0"); // 重置未读消息的标识为0
    ui->widget_5->setVisible(false); // 将未读消息标识设置为不可见

    emit selected(this); // 发送当前item被选中的信号
}

void ChatListItem::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    Selected(true);
}

void ChatListItem::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    if (!m_isChecked)
    {
        Selected(false);
    }
}
