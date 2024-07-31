#ifndef CHATRECORDDIALOG_H
#define CHATRECORDDIALOG_H

#include <QDialog>
#include <Models/friend.h>
#include "historyitem.h"

namespace Ui {
class ChatRecordDialog;
}

class ChatRecordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatRecordDialog(QWidget *parent = nullptr);
    ~ChatRecordDialog();
public slots:
    void slot_showHistroyMsg(Friend*);
private:
    Ui::ChatRecordDialog *ui;
};

#endif // CHATRECORDDIALOG_H
