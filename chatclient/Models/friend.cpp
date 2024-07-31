#include "friend.h"

//Friend::Friend()
//{
//}

Friend::Friend(int wxid, QString name, QString image, bool mute, QString time, int count, QList<Message> messages)
    : User(wxid, name, image)
    , m_mute(mute)
    , m_time(time)
    , m_count(count)
    , m_messages(messages)
{
//    this->_wxid = wxid;
//    this->_name = name;
//    this->_image = image;
//    this->_mute = mute;
//    this->_time = time;
//    this->_count = count;
//    this->_messages = messages;
}

//void Friend::mute(bool mute)
//{
//    this->_mute = mute;
//}

//bool Friend::mute()
//{
//    return this->_mute;
//}

//void Friend::time(QString time)
//{
//    this->_time = time;
//}

//QString Friend::time()
//{
//    return this->_time;
//}

//void Friend::count(int count)
//{
//    this->_count = count;
//}

//int Friend::count()
//{
//    return this->_count;
//}

//void Friend::messages(QList<Message> messages)
//{
//    this->_messages = messages;
//}

//QList<Message> Friend::messages()
//{
//    return this->_messages;
//}
