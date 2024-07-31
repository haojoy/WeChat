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
    void SIG_CloseLoginDialog();

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private slots:
    void on_pb_commit_clicked();

    void on_pb_register_clicked();

    void on_btn_register_clicked();

    void on_btn_backlogin_clicked();

    void on_btn_closedialog_clicked();

private:
    Ui::LoginDialog *ui;
};

#endif // LOGINDIALOG_H
