#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT
signals:
    void SIG_LoginCommit( QString username , QString password);
    void SIG_RegisterCommit( QString username, QString tel, QString password);

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private slots:
    void on_pb_commit_clicked();

    void on_pb_register_clicked();

private:
    Ui::LoginDialog *ui;
};

#endif // LOGINDIALOG_H
