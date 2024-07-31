#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QObject>
#include "Models/message.h"
#include "Models/friend.h"
#include "chatrecorddialog.h"

namespace Ui {
class ChatWidget;
}

class ChatWidget : public QWidget
{
    Q_OBJECT
signals:
    void SIG_SendChatMsg(int id, QString content);
    void SIG_ShowHistoryMsg(Friend*);
    void SIG_SendFile(int friendid, QString filename, uint64_t filesize);
public:
    explicit ChatWidget(QWidget *parent = nullptr, int id = 0);
    ~ChatWidget();

    void setData(Friend *data);

private slots:
    void on_sendBtn_clicked();

    void on_chatRecordBtn_clicked();

    void on_sendFileBtn_clicked();

private:
    Ui::ChatWidget *ui;
    ChatRecordDialog* m_recordDialog;
    int m_id; // TODO:无意义
};

#endif // CHATWIDGET_H
