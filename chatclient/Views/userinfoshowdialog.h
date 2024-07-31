#ifndef USERINFOSHOWDIALOG_H
#define USERINFOSHOWDIALOG_H

#include <QDialog>

namespace Ui {
class UserInfoShowDialog;
}

class UserInfoShowDialog : public QDialog
{
    Q_OBJECT
signals:
    void SIG_AddFriendRequest(QString friendName, int friendId);
public:
    explicit UserInfoShowDialog(QWidget *parent = nullptr);
    ~UserInfoShowDialog();
    void setUserName(QString username);
    void setUserAvatar(QByteArray& byteArray);
    void setUserIcon(QPixmap& pixmap);
    void setUserId(int id) { m_userId = id; }
private slots:
    void on_addFriendBtn_clicked();

private:
    Ui::UserInfoShowDialog *ui;
    /// 该好友信息对应的id
    int m_userId;
};

#endif // USERINFOSHOWDIALOG_H
