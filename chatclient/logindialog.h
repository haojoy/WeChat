#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QMouseEvent>

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

    void showErrorTips(QString text, QString state = "error");
protected:
    //实现窗口可拖动
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

private:

private slots:
    void on_pb_commit_clicked();

    void on_pb_register_clicked();

    void on_btn_register_clicked();

    void on_btn_backlogin_clicked();

    void on_btn_closedialog_clicked();

private:
    Ui::LoginDialog *ui;

    bool dragging;
    QPoint dragStartPosition;
};

#endif // LOGINDIALOG_H
