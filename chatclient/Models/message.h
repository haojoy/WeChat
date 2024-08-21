#ifndef MESSAGE_H
#define MESSAGE_H

#include <QObject>
#include <QDateTime>
#include "Models/model.h"

enum MsgType {
    None = 0,
    Sender = 1,
    Receive = 2,
};

class Message : public Model
{
public:
    explicit Message() = default;
    explicit Message(QString content, QString time, MsgType msgType):
        _content(content), _time(time), _msgType(msgType){}

    void setContent(QString content) { _content = content; }
    QString getContent() const { return _content; }

    void setTime(QString time) { _time = time; }
    QString getTime() const { return _time; }

    void setMsgType(MsgType msgType) { _msgType = msgType; }
    MsgType getMsgType() const { return _msgType; }

private:
    /// 消息内容
    QString _content;
    /// 消息的发送时间
    QString _time;
    /// 消息来源类别
    MsgType _msgType;
};
Q_DECLARE_METATYPE(Message)
Q_DECLARE_METATYPE(Message*)
#endif // MESSAGE_H
