#ifndef USER_H
#define USER_H

#include <QObject>
#include "Models/model.h"

class User : public Model
{
public:
    explicit User() = default;
    explicit User(int wxid, QString name, QString image);

    void setWxid(int wxid) { m_wxid = wxid; }
    int getWxid() const { return m_wxid; }

    void setImage(QString image) { m_image = image; }
    QString getImage() const { return m_image; }

    void setName(QString name) { m_name = name; }
    QString getName() const { return m_name; }

signals:

protected:
    /// 唯一id
    int m_wxid;
    /// 头像
    QString m_image;
    /// 名称
    QString m_name;

};
Q_DECLARE_METATYPE(User)
Q_DECLARE_METATYPE(User*)
#endif // USER_H
