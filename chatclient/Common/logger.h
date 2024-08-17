#ifndef LOGGER_H
#define LOGGER_H

#include <QDebug>
#include <QDateTime>

#define DEBUG qDebug() << "[" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << "][" << __FILE__ << ":" << __LINE__ << "]"

#endif // LOGGER_H
