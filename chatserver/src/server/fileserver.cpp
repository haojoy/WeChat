#include "fileserver.h"
#include "json.hpp"
#include "fileservice.h"

#include <iostream>
#include <functional>
#include <string>
#include <muduo/base/Logging.h>
using namespace std;
using namespace placeholders;
using json = nlohmann::json;

// 初始化聊天服务器对象
FileServer::FileServer(EventLoop *loop,
                       const InetAddress &listenAddr)
    : fileserver_(loop, listenAddr, "FileServer"), fileloop_(loop)
{
    // 注册链接回调
    fileserver_.setConnectionCallback(std::bind(&FileServer::onConnection, this, _1));
    // 注册消息回调
    fileserver_.setMessageCallback(std::bind(&FileServer::onMessage, this, _1, _2, _3));
}

// 启动服务
void FileServer::start()
{
    fileserver_.start();
    LOG_INFO << "[FileServer] started and listening on " << fileserver_.ipPort();
}

// 上报链接相关信息的回调函数
void FileServer::onConnection(const TcpConnectionPtr &conn)
{
    if (!conn->connected()) {
        LOG_ERROR << "[FileServer] disconnected " << conn->peerAddress().toIpPort();
        conn->shutdown();
    }
}

// 上报读写事件相关信息的回调函数
void FileServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buffer,
                           Timestamp time)
{
    while (buffer->readableBytes() >= sizeof(int)) {
        int packSize = *reinterpret_cast<const int*>(buffer->peek());

        if (buffer->readableBytes() < sizeof(int) + packSize) {
            break; // 等待更多数据到达
        }

        buffer->retrieve(sizeof(int)); // 先取出包大小

        // 处理不同类型的数据包
        const char* data = buffer->peek();
        PackType type = *reinterpret_cast<const PackType*>(data);
        auto msgFileHandler = FileService::instance()->getHandler(type);
        msgFileHandler(conn, data, time);
        buffer->retrieve(packSize); // 移除已处理的数据包
    }
}