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
    User(int id = -1, string name = "", string tel = "", string pwd = "", string state = "offline")
    {
        this->id = id;
        this->name = name;
        this->tel = tel;
        this->password = pwd;
        this->state = state;
    }

    void setId(int id) { this->id = id; }
    void setName(string name) { this->name = name; }
    void setTel(string tel) { this->tel = tel; }
    void setPwd(string pwd) { this->password = pwd; }
    void setState(string state) { this->state = state; }

    int getId() { return this->id; }
    string getName() { return this->name; }
    string getTel() { return this->tel; }
    string getPwd() { return this->password; }
    string getState() { return this->state; }

protected:
    int id;
    string name;
    string tel;
    string password;
    string state;
};

#endif