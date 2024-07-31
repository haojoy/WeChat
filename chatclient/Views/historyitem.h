#ifndef HISTORYITEM_H
#define HISTORYITEM_H

#include <QWidget>

namespace Ui {
class HistoryItem;
}

class HistoryItem : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryItem(QWidget *parent = nullptr, QString imageUrl = "", QString name = "");
    ~HistoryItem();
public:
    void setMessage(QString message);
    void setTime(QString time);
private:
    Ui::HistoryItem *ui;
};

#endif // HISTORYITEM_H
