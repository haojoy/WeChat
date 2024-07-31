#ifndef CONTACTWIDGET_H
#define CONTACTWIDGET_H

#include <QWidget>
#include "newfrienditem.h"
#include <map>

namespace Ui {
class ContactWidget;
}

class ContactWidget : public QWidget
{
    Q_OBJECT
signals:
    void SIG_FriendRequestAccept(int id);
public:
    explicit ContactWidget(QWidget *parent = nullptr);
    ~ContactWidget();
    void setItem(int id);
    NewFriendItem* getItem(int id);
    void addNewFriend(int id);
public slots:
    void slot_friendRequestAccepted(int id);
private:
    Ui::ContactWidget *ui;
    std::map<int, NewFriendItem*> m_mapNewFriendItem;
};

#endif // CONTACTWIDGET_H
