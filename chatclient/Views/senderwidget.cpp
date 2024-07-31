#include "senderwidget.h"
#include "ui_senderwidget.h"

SenderWidget::SenderWidget(QWidget *parent, QString imageUrl) :
    QWidget(parent),
    ui(new Ui::SenderWidget)
{
    ui->setupUi(this);
    init(imageUrl);
}

SenderWidget::~SenderWidget()
{
    delete ui;
}

void SenderWidget::setMessage(QString message)
{
    ui->msgLbl->setText(message);
}

void SenderWidget::init(QString imageUrl)
{
    ui->headBtn->setIcon(QIcon(imageUrl));
    ui->checkBtn->setVisible(false);
    connect(ui->msgLbl, &Label::sizeChanged, this, &SenderWidget::sizeChange);
}

void SenderWidget::sizeChange(QRect rect)
{
    auto g_rect = geometry();
    g_rect.setHeight(rect.height() + 20);
    setGeometry(g_rect);
    emit sizeChanged(rect);
}

void SenderWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    ui->msgLbl->setMaximumWidth(width() - 140);
    ui->msgLbl->setText(ui->msgLbl->text());
}
