#ifndef PASSWORDEDIT_H
#define PASSWORDEDIT_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QIcon>

class PasswordEdit : public QWidget
{
    Q_OBJECT

public:
    PasswordEdit(QWidget *parent = nullptr) : QWidget(parent)
    {
        //QHBoxLayout *layout = new QHBoxLayout(this);
        passwordEdit = new QLineEdit(this);
        passwordEdit->setMinimumSize(269, 31);
        passwordEdit->setPlaceholderText("请输入密码");
        passwordEdit->setEchoMode(QLineEdit::Password);

        toggleButton = new QPushButton(QIcon(":/images/eye_off.png"), "", this);
        toggleButton->setCheckable(true);

        toggleButton->setFixedSize(30, 30);
        toggleButton->setStyleSheet(R"(
            QPushButton {
                border: none;
                background: transparent;
            }
        )");

        // 将按钮添加到密码框内
        QHBoxLayout *innerLayout = new QHBoxLayout(this);
        innerLayout->setContentsMargins(0, 0, 0, 0);
        innerLayout->addWidget(passwordEdit);
        innerLayout->addWidget(toggleButton);

        setLayout(innerLayout);


        passwordEdit->setStyleSheet(
            "QLineEdit { padding-right: 35px; }"
            );

        connect(toggleButton, &QPushButton::toggled, this, &PasswordEdit::togglePasswordVisibility);
    }

    QLineEdit* getlineEdit()
    {
        return passwordEdit;
    }

private slots:
    void togglePasswordVisibility(bool checked)
    {
        if (checked)
        {
            passwordEdit->setEchoMode(QLineEdit::Normal);
            toggleButton->setIcon(QIcon(":/images/eye_on.png"));
        }
        else
        {
            passwordEdit->setEchoMode(QLineEdit::Password);
            toggleButton->setIcon(QIcon(":/images/eye_off.png"));
        }
    }

private:
    QLineEdit *passwordEdit;
    QPushButton *toggleButton;
};


#endif // PASSWORDEDIT_H
