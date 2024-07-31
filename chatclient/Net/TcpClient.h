#ifndef __TCPCLIENT_H__
#define __TCPCLIENT_H__
#include "INet.h"
#include <QTcpSocket>
namespace net {
class TcpClient : public INet
{
    Q_OBJECT
public:
    explicit TcpClient(INetMediator* mediator, QObject *parent = nullptr);
    ~TcpClient();
    /**
         * @brief 初始化网络
         * @return 成功返回true, 失败返回false
        */
    bool InitNet() override;
    /**
         * @brief 关闭网络
        */
    void UnInitNet() override;
    /**
         * @brief 发送信息: 同时兼容TCP和UDP
         * @param lSendIP 网络字节序IP地址
         * @param buf 传输数据缓冲区的指针
         * @param nLen 数据长度
         * @return 成功返回true, 失败返回false
        */
    bool SendData(unsigned long lSendIP, const char* buf, int nLen) override;
protected:
    /**
         * @brief 接受信息
        */
    void RecvData() override;
private:

public slots:
    void recvFromSocket();
private:
    /**
         * @brief 主套接字
        */
    QTcpSocket *_socktcp;

    /**
         * @brief 结束判断标志
        */
    bool m_isStop;
};
}
#endif // !__TCPCLIENT_H__



