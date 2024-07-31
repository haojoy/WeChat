#ifndef CONTACTLISTWIDGET_H
#define CONTACTLISTWIDGET_H

#include <QWidget>

namespace Ui {
class ContactListWidget;
}

/**
 * @brief The ContactListWidget class 好友列表
 */
class ContactListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ContactListWidget(QWidget *parent = nullptr);
    ~ContactListWidget();
signals:
    void SIG_GetFriendInfo(QString friendname);
public:
    void modifySearchStackedWidgetIndex();
private slots:
    void on_addBtn_clicked();

    void on_cancelBtn_clicked();

    void on_lineEdit_textChanged(const QString &arg1);

    void on_addFriendBtn_clicked();
protected:
//    /**
//     * @brief mousePressEvent 重写鼠标点击事件
//     * @param event
//     */
//    void mousePressEvent(QMouseEvent *event) override;
private:
    Ui::ContactListWidget *ui;
};

#endif // CONTACTLISTWIDGET_H
