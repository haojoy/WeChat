#ifndef JSONUTILS_H
#define JSONUTILS_H

#include <QObject>
#include <QFile>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonDocument>

class JsonUtils : public QObject
{
    Q_OBJECT

private:
    JsonUtils();

public:
    static QJsonObject getJson(QString path);
};

#endif // JSONUTILS_H
