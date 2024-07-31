#include "historyitem.h"
#include "ui_historyitem.h"

HistoryItem::HistoryItem(QWidget *parent, QString imageUrl, QString name) :
    QWidget(parent),
    ui(new Ui::HistoryItem)
{
    ui->setupUi(this);
    ui->iconBtn->setIcon(QIcon(imageUrl));
    ui->label_name->setText(name);
}

HistoryItem::~HistoryItem()
{
    delete ui;
}


void HistoryItem::setMessage(QString message) {
    ui->label_msg->setText(message);
}
void HistoryItem::setTime(QString time) {
    ui->label_time->setText(time);
}
