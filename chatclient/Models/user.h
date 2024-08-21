#ifndef USER_H
#define USER_H

#include <QObject>
#include "Models/model.h"


class User
{
public:
    // explicit User() = default;

    User(int id = -1, QString name = "", QString image ="", int avatarid = -1, QString tel = "", QString pwd = "", QString state = "offline"):
        id(id), name(name), image(image), avatarid(avatarid), tel(tel), password(pwd), state(state){}

    void setId(int id) { this->id = id; }
    void setAvatarId(int avatarid) { this->avatarid = avatarid; }
    void setImage(QString image) { image = image; }
    void setName(QString name) { this->name = name; }
    void setTel(QString tel) { this->tel = tel; }
    void setPwd(QString pwd) { this->password = pwd; }
    void setState(QString state) { this->state = state; }

    int getId() { return this->id; }
    int getAvatarId() { return this->avatarid; }
    QString getImage() const { return image; }
    QString getName() { return this->name; }
    QString getTel() { return this->tel; }
    QString getPwd() { return this->password; }
    QString getState() { return this->state; }

protected:
    int id;
    QString name;
    QString image;
    int avatarid;
    QString tel;
    QString password;
    QString state;
};

Q_DECLARE_METATYPE(User)
Q_DECLARE_METATYPE(User*)
#endif // USER_H
