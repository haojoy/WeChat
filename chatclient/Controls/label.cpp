#include "label.h"
#include <QtMath>

Label::Label(QWidget *parent) : QLabel(parent)
{
    setWordWrap(true);
}

void Label::setText(QString text)
{
    QLabel::setText(text);
    double maxWidth = maximumWidth();
    auto width = fontMetrics().boundingRect(text).width();
    auto height = fontMetrics().height();
    QRect g_rect = geometry();
    if (width + 20 >= maxWidth)
    {
        maxWidth = maxWidth - 20;
        auto ratio = ceil(width / maxWidth);
        g_rect.setHeight(height * ratio + 30);
        g_rect.setWidth(maxWidth);
    }
    else
    {
        g_rect.setWidth(width + 20);
    }
    setGeometry(g_rect);
    emit sizeChanged(g_rect);
}
