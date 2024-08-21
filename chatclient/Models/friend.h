#ifndef FRIEND_H
#define FRIEND_H

#include <QObject>
#include <QString>
#include "user.h"
#include "message.h"

class Friend : public User
{
public:
    explicit Friend() = default;
    explicit Friend(int wxid, QString name, QString image, bool mute, QString time, int count, QList<Message> messages):
        User(wxid, name, image), m_mute(mute), m_time(time), m_count(count), m_messages(messages){}


    void setMute(bool mute) { m_mute = mute; }
    bool getMute() const { return m_mute; }

    void setTime(QString time) { m_time = time; }
    QString getTime() const { return m_time; }

    void setCount(int count) { m_count = count; }
    int getCount() const { return m_count; }
    void addCount(int num) { m_count += num; }

    void setMessages(QList<Message> messages) { m_messages = messages; }
    QList<Message> getMessages() const { return  m_messages; }
    void addMessage(Message& message) {m_messages.push_back(message);}
protected:
    /// 表示好友是否被静音的状态
    bool m_mute;
    /// 好友最后一条消息的时间
    QString m_time;
    /// 未读消息数量
    int m_count;
    /// 好友的消息列表
    QList<Message> m_messages;
};
Q_DECLARE_METATYPE(Friend)
Q_DECLARE_METATYPE(Friend*)
#endif // FRIEND_H
