#ifndef CHATLISTWIDGET_H
#define CHATLISTWIDGET_H

#include <QWidget>
#include <QVariant>
#include "chatlistitem.h"
#include "Models/friend.h"

#include <map>

enum PageType {
    AllChatView,
    ContactView,
    CollectView,
};

namespace Ui {
class ChatListWidget;
}

/**
 * @brief The ChatListWidget class 聊天列表
 */
class ChatListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatListWidget(QWidget *parent = nullptr);
    ~ChatListWidget();

    /**
     * @brief AddItem 添加聊天项
     * @param data 聊天好友
     */
    void AddItem(Friend *data);
    /**
     * @brief isContainsKey 好友映射表中是否存在id好友
     * @param id
     * @return
     */
    bool isContainsKey(int id);
    /**
     * @brief getItemById 获取id对应的item
     * @param id
     * @return
     */
    ChatListItem* getItemById(int id);

    static ChatListItem* GetItem() { return m_item; }
private:
    Ui::ChatListWidget *ui;
    /// 状态变量: 当前被选中的聊天项item
    static ChatListItem *m_item;
    /// 好友id与好友列表项的映射关系map
    std::map<int, ChatListItem*> m_mapIdToChatItem;
private slots:
    /**
     * @brief onItemSelected 当聊天列表项被选中后对应的处理槽函数
     * @param item
     */
    void onItemSelected(ChatListItem *item);

signals:
    /**
     * @brief onItemClicked 点击聊天项
     * @param var
     * @param type
     */
    void onItemClicked(QVariant var, PageType type);
};

#endif // CHATLISTWIDGET_H
