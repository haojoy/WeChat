#ifndef USER_H
#define USER_H

#include <string>
using namespace std;

#include "connection.h"
#include "connectionpool.h"

// User表的ORM类
class User
{
public:
    //User(int id ,string state): id(id),state(state){}
    User(int id = -1, string avatarid = "", string name = "", string tel = "", string pwd = "", string state = "offline"):
        id(id), avatarid(avatarid), name(name), tel(tel), password(pwd), state(state){}

    void setId(int id) { this->id = id; }
    void setAvatarId(string avatarid) { this->avatarid = avatarid; }
    void setName(string name) { this->name = name; }
    void setTel(string tel) { this->tel = tel; }
    void setPwd(string pwd) { this->password = pwd; }
    void setState(string state) { this->state = state; }

    int getId() { return this->id; }
    string getAvatarId() { return this->avatarid; }
    string getName() { return this->name; }
    string getTel() { return this->tel; }
    string getPwd() { return this->password; }
    string getState() { return this->state; }

protected:
    int id;
    string avatarid;
    string name;
    string tel;
    string password;
    string state;
};

#endif