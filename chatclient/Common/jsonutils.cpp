#include "jsonutils.h"

JsonUtils::JsonUtils()
{

}

QJsonObject JsonUtils::getJson(QString path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        return QJsonObject();
    }
    QByteArray data(file.readAll());
    file.close();
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError)
    {
        return QJsonObject();
    }
    return document.object();
}
