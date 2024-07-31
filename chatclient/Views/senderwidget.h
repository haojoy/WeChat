#ifndef SENDERWIDGET_H
#define SENDERWIDGET_H

#include <QWidget>

namespace Ui {
class SenderWidget;
}

class SenderWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SenderWidget(QWidget *parent = nullptr, QString imageUrl = "");
    ~SenderWidget();

    void setMessage(QString message);

signals:
    void sizeChanged(QRect rect);

private:
    Ui::SenderWidget *ui;

    void init(QString imageUrl);
    void sizeChange(QRect rect);

protected:
    void resizeEvent(QResizeEvent *event);
};

#endif // SENDERWIDGET_H
