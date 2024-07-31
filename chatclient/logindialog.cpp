#include "logindialog.h"
#include "ui_logindialog.h"
#include "QMessageBox"
#include <QFile>

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    //setWindowTitle("登录&注册");

    // 加载qss样式表
    QFile file(":/qss/style.qss");
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(file.readAll());
        this->setStyleSheet(styleSheet);
        file.close();
    } else {
        QMessageBox::warning(this, "Error", "Failed to load the style sheet file.");
    }
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

/**
 * @brief LoginDialog::on_pb_commit_clicked 登录按钮点击事件
 */
void LoginDialog::on_pb_commit_clicked()
{
    QString username = ui->lineEdit_username->text();
    QString password = ui->lineEdit_password->text();
    // 1. 首先判断输入是否合法（前端）
    if(username.isEmpty() || password.isEmpty()) { // 判断输入是否为空
        QMessageBox::about(this, "提示", "用户名或密码不能为空");
        return;
    }
    Q_EMIT SIG_LoginCommit(username, password);
}

/**
 * @brief LoginDialog::on_pb_register_clicked 注册按钮点击事件
 */
void LoginDialog::on_pb_register_clicked()
{
    QString username = ui->lineEdit_username_register->text();
    QString tel = ui->lineEdit_tel_register->text();
    QString password = ui->lineEdit_password_register->text();
    QString password_confirm = ui->lineEdit_password_confirm->text();
    // (前端)判断表单是否合法
    if (username.isEmpty() || tel.isEmpty() || password.isEmpty() || password_confirm.isEmpty()) { // 判断用户输入是否为空
        QMessageBox::about(this, "提示", "不能为空");
        return;
    }
    if (password.compare(password_confirm) != 0) {
        QMessageBox::about(this, "提示", "两次密码输入不一致");
        return;
    }
    Q_EMIT SIG_RegisterCommit(username, tel, password);
}


void LoginDialog::on_btn_register_clicked()
{
    ui->tab_page->setCurrentWidget(ui->page_register);
}


void LoginDialog::on_btn_backlogin_clicked()
{
    ui->tab_page->setCurrentWidget(ui->page_login);
}


void LoginDialog::on_btn_closedialog_clicked()
{
    Q_EMIT SIG_CloseLoginDialog();
}

