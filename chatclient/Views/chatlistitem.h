#ifndef CHATLISTITEM_H
#define CHATLISTITEM_H

#include <QWidget>
#include <QStringLiteral>
#include "Models/friend.h"
#include <vector>

namespace Ui {
class ChatListItem;
}

class ChatListItem : public QWidget
{
    Q_OBJECT

public:
    explicit ChatListItem(QWidget *parent = nullptr);
    ~ChatListItem();

    /**
     * @brief Checked 更新是否被点击状态m_isChecked
     * @param checked
     */
    void Checked(bool checked);
    /**
     * @brief Selected 设置聊天列表项的选中状态
     * @param selected
     */
    void Selected(bool selected);

    Friend *getItem() const { return m_data; }
    void setItem(Friend *data);
    /**
     * @brief setContent 设置聊天列表项的内容
     * @param content
     */
    void setContent(QString content);
    /**
     * @brief updateContent 更新聊天列表项
     * @param message
     */
    void updateContent(Message& messages);
    /**
     * @brief updateFriend 更新本地聊天记录
     * @param message
     */
    void updateFriend(Message& message);
private:
    /**
     * @brief mousePressEvent 鼠标按下聊天列表项触发
     * @param event
     */
    void mousePressEvent(QMouseEvent *event) override;
    /**
     * @brief enterEvent 鼠标进入该聊天列表项时更新select状态
     * @param event
     */
    void enterEvent(QEvent *event) override;
    /**
     * @brief leaveEvent 鼠标离开聊天列表项时更新select状态
     * @param event
     */
    void leaveEvent(QEvent *event) override;
private:
    Ui::ChatListItem *ui;
    ///
    Friend *m_data;
    /// 该item是否被点击
    bool m_isChecked;
signals:
    /// 当前item被选中的信号
    void selected(ChatListItem *item);
};
Q_DECLARE_METATYPE(ChatListItem*)
#endif // CHATLISTITEM_H
