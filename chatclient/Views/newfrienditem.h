#ifndef NEWFRIENDITEM_H
#define NEWFRIENDITEM_H

#include <QWidget>

namespace Ui {
class NewFriendItem;
}

class NewFriendItem : public QWidget
{
    Q_OBJECT
signals:
    void SIG_FriendRequestAccepted(int id);
public:
    explicit NewFriendItem(QWidget *parent = nullptr);
    ~NewFriendItem();
    void setId(int id) { m_friendId = id;}
    int getId() const { return m_friendId; }
    void setName(QString name) { m_friendName = name; }
    QString& getName() { return m_friendName; }
    void setIcon(QPixmap& pixmap) { m_friendIcon = pixmap; }
    void setInfo();
private slots:
    void on_acceptBtn_clicked();
private:
    Ui::NewFriendItem *ui;
    int m_friendId;
    QString m_friendName;
    QPixmap m_friendIcon;
};

#endif // NEWFRIENDITEM_H
