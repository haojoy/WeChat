#include "chatlistwidget.h"
#include "ui_chatlistwidget.h"
#include "chatlistitem.h"
#include <QVariant>


ChatListItem* ChatListWidget::m_item = nullptr;
ChatListWidget::ChatListWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatListWidget)
{
    ui->setupUi(this);
}

ChatListWidget::~ChatListWidget()
{
    delete ui;
}

void ChatListWidget::AddItem(Friend *data)
{
    auto item = new QListWidgetItem;            // 创建列表项对象
    QVariant var = QVariant::fromValue(data);   // 将Friend* data转换为QVariant类型
    item->setData(Qt::UserRole, var);           // 将转换后的 QVariant 数据存储在列表项的 Qt::UserRole 角色中
    item->setSizeHint(QSize(250, 65));          // 设置列表项的大小提示
    ui->listWidget->addItem(item);              // 将列表项添加到listWidget

    auto widget = new ChatListItem;             // 创建 ChatListItem 对象
    widget->setItem(data);                      // 设置 item对象Widget
    m_mapIdToChatItem[data->getWxid()] = widget;// 设置id与item的映射
    ui->listWidget->setItemWidget(item, widget);// 将 widget 设置为列表项的小部件，以便在列表中显示自定义的聊天项内容

    connect(widget, &ChatListItem::selected, this, &ChatListWidget::onItemSelected);
}

void ChatListWidget::onItemSelected(ChatListItem *item)
{
    if (item == nullptr)
    {
        return;
    }
    if (m_item != nullptr)
    {
        m_item->Selected(false); // 取消前一个被选中的item的选中状态
        m_item->Checked(false);  // 取消前一个被选中的item的点中状态
    }
    item->Selected(true);
    item->Checked(true);
    m_item = item;

    QVariant var = QVariant::fromValue(item->getItem());   //
    emit onItemClicked(var, PageType::AllChatView);     // 发送当前item被点中信号
}

bool ChatListWidget::isContainsKey(int id) {
    return m_mapIdToChatItem.find(id) != m_mapIdToChatItem.end();
}

ChatListItem* ChatListWidget::getItemById(int id) {
    if(!isContainsKey(id)) return nullptr;
    return m_mapIdToChatItem[id];
}
