#ifndef OFFLINEMSG_H
#define OFFLINEMSG_H

#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
using namespace std;

// OffLineMsg表的ORM类
class OffLineMsg
{
public:
    OffLineMsg(int id = -1, int sendTo = -1, int sendFrom = -1, string messageContent = "", string messageType = "", string createTime = "")
    : id(id), sendTo(sendTo), sendFrom(sendFrom), messageContent(messageContent), messageType(messageType), createTime(createTime) {}

    void setId(int id) { this->id = id; }
    void setSendTo(int sendTo) { this->sendTo = sendTo; }
    void setSendFrom(int sendFrom) { this->sendFrom = sendFrom; }
    void setMsgContent(string messageContent) { this->messageContent = messageContent; }
    void setMsgType(string messageType) { this->messageType = messageType; }
    void setCreateTime() {
        auto now = chrono::system_clock::now();
        time_t now_time = chrono::system_clock::to_time_t(now);
        tm local_tm = *localtime(&now_time);
        
        stringstream ss;
        ss << put_time(&local_tm, "%Y-%m-%d %H:%M:%S");
        this->createTime = ss.str();
    }

    int getId() {return this->id; }
    int getSendTo() {return this->sendTo; }
    int getSendFrom() { return this->sendFrom;}
    string getMsgContent() { return this->messageContent; }
    string getMsgtype() { return this->messageType; }
    string getCreateTime() { return this->createTime; }

protected:
    int id;
    int sendTo;
    int sendFrom;
    string messageContent;
    string messageType;
    string createTime;
};

#endif