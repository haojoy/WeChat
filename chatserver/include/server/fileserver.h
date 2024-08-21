#ifndef __FILESERVER_H__
#define __FILESERVER_H__

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <map>
#include <memory>
#include <string>
#include <cstdio>
using namespace std;
using namespace placeholders;
using namespace muduo;
using namespace muduo::net;

class FileServer
{
public:
    FileServer(EventLoop *loop, const InetAddress &listenAddr);
    void start();

private:
    void onConnection(const TcpConnectionPtr &);
    void onMessage(const TcpConnectionPtr &, Buffer *, Timestamp);

private:
    TcpServer fileserver_; 
    EventLoop *fileloop_;  
};



#endif // __FILESERVER_H__