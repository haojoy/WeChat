#include "chatserver.hpp"
#include "chatservice.hpp"
#include "fileserver.h"
#include "fileservice.h"
#include <iostream>
#include <signal.h>
using namespace std;

// 处理服务器ctrl+c结束后，重置user的状态信息
void resetHandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}

int main(int argc, char **argv)
{
    /*
    if (argc < 3)
    {
        cerr << "command invalid! example: ./ChatServer 127.0.0.1 6000" << endl;
        exit(-1);
    }

    // 解析通过命令行参数传递的ip和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    signal(SIGINT, resetHandler);

    EventLoop loop;
    InetAddress addr(ip, port);
    ChatServer server(&loop, addr, "ChatServer");

    server.start();
    loop.loop();
    */
    signal(SIGINT, resetHandler);
    // 创建两个线程来运行不同的 EventLoop
    std::thread chatThread([]() {
        EventLoop loopchat;
        InetAddress chatAddr(8080);
        ChatServer chatServer(&loopchat, chatAddr);
        chatServer.start();
        loopchat.loop();
    });

    std::thread fileThread([]() {
        EventLoop loopfile;
        InetAddress fileAddr(8088);
        FileServer fileServer(&loopfile, fileAddr);
        fileServer.start();
        loopfile.loop();
    });

    // 等待两个线程结束
    chatThread.join();
    fileThread.join();

    return 0;
}