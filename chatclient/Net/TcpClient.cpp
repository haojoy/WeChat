#include "TcpClient.h"
#include <iostream>
#include <QNetworkProxy>
#include <QDataStream>
#include <QtEndian>
#include <QThread>
#include "qDebug"
namespace net {
TcpClient::TcpClient(INetMediator* mediator, QObject *parent)
    : INet(parent), _socktcp(nullptr), m_isStop(false)
{
    m_pMediator = mediator;
    qRegisterMetaType<QAbstractSocket::SocketError>();
}
TcpClient::~TcpClient()
{
    UnInitNet();
}
bool TcpClient::InitNet()
{
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::NoProxy);
    QNetworkProxy::setApplicationProxy(proxy);
    if(_socktcp == nullptr)
        _socktcp = new QTcpSocket(this); //tcp
    _socktcp->connectToHost(_DEF_SERVER_IP, _DEF_TCP_PORT);
    connect(_socktcp, SIGNAL(readyRead()), this, SLOT(recvFromSocket()), Qt::UniqueConnection); //接受数据
    //处理套接字错误
    connect(_socktcp, &QTcpSocket::errorOccurred,
        this, [this](QAbstractSocket::SocketError error) {
            qDebug() << "socket error:" << error;
        },
        Qt::UniqueConnection);


    if(_socktcp->waitForConnected())
    {
        return true;
    }
    UnInitNet();
    _socktcp->close();
    return false;
}
void TcpClient::UnInitNet()
{
    m_isStop = true;
    if (_socktcp && _socktcp->isOpen())
    {
        _socktcp->close();
    }

}
void printBuffer(const char* buf, int nLen) {
    // 确保 buf 不为空
    if (buf == nullptr || nLen <= 0) {
        qDebug() << "Buffer is empty or length is non-positive.";
        return;
    }

    // 输出为字符串（适用于以 null 结尾的字符串）
    qDebug() << "Buffer as string:" << QString::fromUtf8(buf);

    // 输出为十六进制格式
    qDebug() << "Buffer as hex:" << QByteArray(buf, nLen).toHex();

    // 逐字节输出
    for (int i = 0; i < nLen; ++i) {
        qDebug() << "Byte" << i << ":" << static_cast<unsigned char>(buf[i]);
    }
}

bool TcpClient::SendData(unsigned long lSendIP, const char* buf, int nLen)
{
    if (!buf || nLen <= 0)
        return false;

    // 确保套接字连接状态
    if (_socktcp->state() != QAbstractSocket::ConnectedState)
        return false;

    // 发送数据包大小
    QByteArray sizeData(reinterpret_cast<const char*>(&nLen), sizeof(int));
    if (_socktcp->write(sizeData) != sizeof(int)) {
        qDebug() << "Failed to send packet size";
        return false;
    }

    // 发送数据包
    QByteArray data(buf, nLen);
    if (_socktcp->write(data) != nLen) {
        qDebug() << "Failed to send packet data";
        return false;
    }

    // 确保数据全部发送完毕
    if (!_socktcp->waitForBytesWritten(3000)) { // 等待数据被写入，超时时间为 3000ms
        qDebug() << "Failed to write all data";
        return false;
    }

    return true;
}

void TcpClient::RecvData() {

}


void TcpClient::recvFromSocket()
{
    QDataStream in(_socktcp);
    in.setByteOrder(QDataStream::LittleEndian);

    while (_socktcp->bytesAvailable() >= static_cast<int>(sizeof(qint32))) {
        qint32 nPackSize = 0;
        in >> nPackSize;

        if (in.status() != QDataStream::Ok || nPackSize <= 0) {
            qDebug() << "Failed to read packet size or invalid packet size:" << nPackSize;
            continue;
        }

        // 等待接收完整数据包
        while (_socktcp->bytesAvailable() < nPackSize) {
            if (!_socktcp->waitForReadyRead(100)) {
                qDebug() << "Waiting for more data...";
            }
        }

        QByteArray recvbuf(nPackSize, 0);
        qint64 bytesRead = 0;
        qint64 totalBytesRead = 0;

        while (totalBytesRead < nPackSize && !m_isStop) {
            bytesRead = _socktcp->read(recvbuf.data() + totalBytesRead, nPackSize - totalBytesRead);

            if (bytesRead > 0) {
                totalBytesRead += bytesRead;
                qDebug() << "Received" << bytesRead << "bytes, remaining" << (nPackSize - totalBytesRead);
            } else if (bytesRead == 0) {
                qDebug() << "Connection closed by peer";
                break;
            } else {
                qDebug() << "Failed to read data:" << _socktcp->errorString();
                break;
            }
        }

        if (totalBytesRead == nPackSize) {
            qDebug() << "Received data:" << recvbuf;
            m_pMediator->DealData(0, recvbuf.constData(), recvbuf.size());
        }
    }
}


}

