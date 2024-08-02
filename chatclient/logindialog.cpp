#include "logindialog.h"
#include "ui_logindialog.h"
#include "QMessageBox"
#include <QFile>
#include <QStyle>

std::function<void(QWidget *)> flushtips = [](QWidget *w) {
    QStyle *style = w->style();
    style->unpolish(w);
    style->polish(w);
};

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

void LoginDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        dragging = true;
        dragStartPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void LoginDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPos() - dragStartPosition);
        event->accept();
        setCursor(Qt::ClosedHandCursor);
    }
}

void LoginDialog::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        dragging = false;
        event->accept();
        setCursor(Qt::ArrowCursor);
    }
}

void LoginDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        if (ui->tab_page->currentWidget() == ui->page_login)
        {
            on_pb_commit_clicked();
        }
        else if (ui->tab_page->currentWidget() == ui->page_register)
        {
            on_pb_register_clicked();
        }
        event->accept();
    }
    else
    {
        QDialog::keyPressEvent(event);
    }
}

void LoginDialog::showErrorTips(QString text, QString state)
{
    ui->label_tip->setProperty("state", state);
    ui->label_tip->setText(text);
    flushtips(ui->label_tip);
}

/**
 * @brief LoginDialog::on_pb_commit_clicked 登录按钮点击事件
 */
void LoginDialog::on_pb_commit_clicked()
{
    // QString username = ui->lineEdit_username->text();
    // QString password = ui->lineEdit_password->text();

    QString username = ui->lineEdit_username->text();
    QString password = ui->pwd_lineedit->getlineEdit()->text();
    if (username.isEmpty())
    {
        showErrorTips("用户名不能为空!");
        ui->lineEdit_username->setFocus();
        return;
    }

    if (password.isEmpty())
    {
        showErrorTips("密码不能为空!");
        ui->pwd_lineedit->getlineEdit()->setFocus();
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
    QString password = ui->pwd_lineedit2->getlineEdit()->text();
    QString password_confirm = ui->pwd_lineedit3->getlineEdit()->text();

    if (username.isEmpty())
    {
        showErrorTips("用户名不能为空!");
        ui->lineEdit_username->setFocus();
        return;
    }
    if (tel.isEmpty())
    {
        showErrorTips("电话号码不能为空!");
        ui->lineEdit_tel_register->setFocus();
        return;
    }
    if (password.isEmpty())
    {
        showErrorTips("密码不能为空!");
        ui->pwd_lineedit2->getlineEdit()->setFocus();
        return;
    }
    if (password_confirm.isEmpty())
    {
        showErrorTips("密码不能为空!");
        ui->pwd_lineedit3->getlineEdit()->setFocus();
        return;
    }
    if (password.compare(password_confirm) != 0) {
        showErrorTips("两次密码输入不一致!");
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

