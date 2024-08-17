#include "TcpClient.h"
#include <iostream>
#include <QNetworkProxy>
#include <QDataStream>
#include <QtEndian>
#include <QThread>
#include <unordered_map>
#include "qDebug"

std::unordered_map<QAbstractSocket::SocketError, QString> netErrorMap = {
    {QAbstractSocket::ConnectionRefusedError, "Connection Refused Error"},
    {QAbstractSocket::RemoteHostClosedError, "Remote Host Closed Error"},
    {QAbstractSocket::HostNotFoundError, "Host Not Found Error"},
    {QAbstractSocket::SocketAccessError, "Socket Access Error"},
    {QAbstractSocket::SocketResourceError, "Socket Resource Error"},
    {QAbstractSocket::SocketTimeoutError, "Socket Timeout Error"},
    {QAbstractSocket::DatagramTooLargeError, "Datagram Too Large Error"},
    {QAbstractSocket::NetworkError, "Network Error"},
    {QAbstractSocket::AddressInUseError, "Address In Use Error"},
    {QAbstractSocket::SocketAddressNotAvailableError, "Socket Address Not Available Error"},
    {QAbstractSocket::UnsupportedSocketOperationError, "Unsupported Socket Operation Error"},
    {QAbstractSocket::UnfinishedSocketOperationError, "Unfinished Socket Operation Error"},
    {QAbstractSocket::ProxyAuthenticationRequiredError, "Proxy Authentication Required Error"},
    {QAbstractSocket::SslHandshakeFailedError, "SSL Handshake Failed Error"},
    {QAbstractSocket::ProxyConnectionRefusedError, "Proxy Connection Refused Error"},
    {QAbstractSocket::ProxyConnectionClosedError, "Proxy Connection Closed Error"},
    {QAbstractSocket::ProxyConnectionTimeoutError, "Proxy Connection Timeout Error"},
    {QAbstractSocket::ProxyNotFoundError, "Proxy Not Found Error"},
    {QAbstractSocket::ProxyProtocolError, "Proxy Protocol Error"},
    {QAbstractSocket::OperationError, "Operation Error"},
    {QAbstractSocket::SslInternalError, "SSL Internal Error"},
    {QAbstractSocket::SslInvalidUserDataError, "SSL Invalid User Data Error"},
    {QAbstractSocket::TemporaryError, "Temporary Error"},
    {QAbstractSocket::UnknownSocketError, "Unknown Socket Error"}
};

namespace net {
TcpClient::TcpClient(INetMediator* mediator, QObject *parent)
    : INet(parent), _socktcp(nullptr), m_isStop(false),retryCount(0)
{
    retryTimer = new QTimer(this);
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

    connect(_socktcp, &QTcpSocket::readyRead, this, &TcpClient::recvFromSocket, Qt::UniqueConnection);
    connect(_socktcp, &QTcpSocket::errorOccurred, this, &TcpClient::handleSocketError, Qt::UniqueConnection);
    connect(_socktcp, &QTcpSocket::connected, this, &TcpClient::handleConnected, Qt::UniqueConnection);
    connect(_socktcp, &QTcpSocket::disconnected, this, &TcpClient::handleDisconnected, Qt::UniqueConnection);
    connect(retryTimer, &QTimer::timeout, this, &TcpClient::attemptReconnect);

    retryTimer->setInterval(5000);
    retryTimer->setSingleShot(true);
    return attemptReconnect();

    // UnInitNet();
    // _socktcp->close();
    // return false;
}

bool TcpClient::attemptReconnect()
{
    if (retryCount >= maxRetries) {
        retryTimer->stop();
        retryCount = 0;
        m_pMediator->reportNetworkStatus("服务器连接失败! 请稍后重试", Error);
        return false;
    }

    if (_socktcp->state() != QAbstractSocket::ConnectedState) {
        m_pMediator->reportNetworkStatus("请稍等! 正在连接服务器...", Normal);
        _socktcp->connectToHost(_DEF_SERVER_IP, _DEF_TCP_PORT);
        retryCount++;
        if (_socktcp->waitForConnected(3000)) {
            retryCount = 0;
            return true;
        }
        return false;
    }
    return true;
}

void TcpClient::handleSocketError(QAbstractSocket::SocketError error)
{
    // DEBUG << "socket error:" << error;
    QString errorString;
    auto it = netErrorMap.find(error);
    if (it != netErrorMap.end()) {
        errorString = it->second;
    } else {
        errorString = "Unknown Socket Error";
    }
    // m_pMediator->reportNetworkStatus(errorString, Error);
    if (!retryTimer->isActive() && retryCount <= maxRetries) {
        retryTimer->start();
    }
}

void TcpClient::handleConnected()
{
    m_pMediator->reportNetworkStatus("服务器连接成功!", Normal);
    retryTimer->stop();
    retryCount = 0;
}

void TcpClient::handleDisconnected()
{
    if (!retryTimer->isActive() && retryCount <= maxRetries) {
        retryTimer->start();
    }
}

void TcpClient::UnInitNet()
{
    if (_socktcp) {
        if (_socktcp->isOpen()) {
            _socktcp->close();
        }
        _socktcp->disconnectFromHost();
        if (_socktcp->state() != QAbstractSocket::UnconnectedState) {
            _socktcp->waitForDisconnected();
        }
        _socktcp->deleteLater();
        _socktcp= nullptr;
    }

    if (retryTimer) {
        retryTimer->stop();
        retryTimer->deleteLater();
        retryTimer = nullptr;
    }

}
void printBuffer(const char* buf, int nLen) {
    // 确保 buf 不为空
    if (buf == nullptr || nLen <= 0) {
        DEBUG << "Buffer is empty or length is non-positive.";
        return;
    }

    // 输出为字符串（适用于以 null 结尾的字符串）
    DEBUG << "Buffer as string:" << QString::fromUtf8(buf);

    // 输出为十六进制格式
    DEBUG << "Buffer as hex:" << QByteArray(buf, nLen).toHex();

    // 逐字节输出
    for (int i = 0; i < nLen; ++i) {
        DEBUG << "Byte" << i << ":" << static_cast<unsigned char>(buf[i]);
    }
}

bool TcpClient::SendData(unsigned long lSendIP, const char* buf, int nLen)
{
    Q_UNUSED(lSendIP);

    if (!buf || nLen <= 0)
        return false;

    // 确保套接字连接状态
    if (_socktcp->state() != QAbstractSocket::ConnectedState)
        return false;

    // // 发送数据包大小
    // QByteArray sizeData(reinterpret_cast<const char*>(&nLen), sizeof(int));
    // if (_socktcp->write(sizeData) != sizeof(int)) {
    //     DEBUG << "Failed to send packet size";
    //     return false;
    // }

    // 发送数据包
    QByteArray data(buf, nLen);
    if (_socktcp->write(data) != nLen) {
        DEBUG << "Failed to send packet data";
        return false;
    }

    // 确保数据全部发送完毕
    if (!_socktcp->waitForBytesWritten(3000)) { // 等待数据被写入，超时时间为 3000ms
        DEBUG << "Failed to write all data";
        return false;
    }

    return true;
}

void TcpClient::RecvData() {

}


void TcpClient::recvFromSocket()
{
    QByteArray jsonData = _socktcp->readAll();
    string jsonString = jsonData.toStdString();
    DEBUG << jsonString.c_str();
    m_pMediator->DealData(0, jsonString.c_str(), jsonString.size());
}


}

