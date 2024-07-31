#ifndef __TCPCLIENTMEDIATOR_H__
#define __TCPCLIENTMEDIATOR_H__
#include "INetMediator.h"
namespace net {
class TcpClientMediator : public INetMediator
{
    Q_OBJECT
public:
    TcpClientMediator();
    ~TcpClientMediator();
public:
    /**
         * @brief 打开网络
         * @return 成功返回true; 失败返回false
        */
    bool OpenNet() override;
    /**
         * @brief 关闭网络
        */
    void CloseNet() override;
    /**
         * @brief 发送数据
         * @param lSendIP 网络字节序IP地址
         * @param buf 数据缓冲区指针
         * @param nLen 数据大小
         * @return 成功返回true; 失败返回false
        */
    bool SendData(unsigned long lSendIP, const char* buf, int nLen) override;
    /**
         * @brief 处理数据
         * @param lSendIP 网络字节序IP地址
         * @param buf 数据缓冲区指针
         * @param nLen 数据大小
        */
    void DealData(unsigned long lSendIP, const char* buf, int nLen) override;
};
}
#endif // !__TCPCLIENTMEDIATOR_H__
