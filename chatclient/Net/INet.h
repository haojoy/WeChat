#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <process.h>
#include "packdef.h"
#include "INetMediator.h"
#include <QObject>
// #pragma comment (lib, "ws2_32.lib")

constexpr const char* Normal = "normal";
constexpr const char* Error = "error";

namespace net {
class INet:public QObject
{
    Q_OBJECT
public:
    explicit INet(QObject *parent) : QObject(parent) {}
    virtual ~INet() {}
    virtual bool InitNet() = 0;
    virtual void UnInitNet() = 0;
    virtual bool SendData(unsigned long lSendIP, const char* buf, int nLen) = 0;
protected:
    virtual void RecvData() = 0;
    INetMediator* m_pMediator;

};
}

