#include "fileutil.h"
#include <QFile>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QVariant>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QTextCodec>
#include "Net/INet.h"
#include "fileTransferProtocol.h"
#include "qtcpsocket.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>
#include <QByteArray>
#include <QDebug>
#include "logger.h"

QSettings* FileUtil::m_userIconSetting = nullptr;
QSettings* FileUtil::m_userFileSetting = nullptr;
QMap<QString, QString> FileUtil::m_mapFileMd5ToPath;


void FileUtil::initializeFileMd5Map() {
    QSettings userIconSetting("./UserIconConfig.ini", QSettings::IniFormat);

    QStringList keys = userIconSetting.allKeys();
    for (const QString& key : keys) {
        if (key.endsWith("/filePath")) {
            // 提取文件名
            QString encodedFileName = key.section('/', 0, 0);
            QString fileName = QUrl::fromPercentEncoding(encodedFileName.toUtf8());

            // 获取文件路径和MD5值
            QString filePath = userIconSetting.value(key).toString();
            QString fileMd5 = userIconSetting.value(QString("%1/MD5").arg(encodedFileName)).toString();

            if (!fileMd5.isEmpty()) {
                m_mapFileMd5ToPath[fileMd5] = filePath;
            }
        }
    }
}

FileUtil::FileUtil()
{
    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    m_userIconSetting = new QSettings("./UserIconConfig.ini", QSettings::IniFormat);
    m_userFileSetting = new QSettings("./UserFileConfig.ini", QSettings::IniFormat);
    m_userIconSetting->setIniCodec(codec);
    m_userFileSetting->setIniCodec(codec);
    m_userIconSetting->sync();
    m_userFileSetting->sync();
    initializeFileMd5Map();
}

void FileUtil::insertMD5IntoConfig(QString&& filePath)
{
    QFile file(filePath);
    QFileInfo fileInfo(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QCryptographicHash hash(QCryptographicHash::Md5);
        if (hash.addData(&file)) {
            QByteArray md5Sum(hash.result());
            // 向配置文件中写入数据
            m_userIconSetting->setValue(QString("%1/filePath").arg(fileInfo.fileName()), filePath);
            m_userIconSetting->setValue(QString("%1/MD5").arg(fileInfo.fileName()), QString(md5Sum.toHex()));
        }
        file.close();
    }
}

void FileUtil::insertMD5IntoConfig(QString &filePath, QString& fileMd5)
{
    QFile file(filePath);
    QFileInfo fileInfo(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QCryptographicHash hash(QCryptographicHash::Md5);
        if (hash.addData(&file)) {
            QByteArray md5Sum(hash.result());
            fileMd5 = md5Sum.toHex();
            // 向配置文件中写入数据
            m_userIconSetting->setValue(QString("%1/filePath").arg(fileInfo.fileName()), filePath);
            m_userIconSetting->setValue(QString("%1/MD5").arg(fileInfo.fileName()), fileMd5);
        }
        file.close();
    }
}
#include <iostream>
void FileUtil::download(const char *remoteFilePath, int fileSize, const char* fileId, const char* fileMd5, int uid)
{
    // 判断本地是否有该头像
    // 1. 检查某个键是否存在
    if (m_mapFileMd5ToPath.find(fileMd5) != m_mapFileMd5ToPath.end()) {
        QString filePath = m_mapFileMd5ToPath[fileMd5];
        if (QFile::exists(filePath)) { // 文件存在
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray byteArray = file.readAll();
                file.close();
                return;
            }
        }
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    /*连接connect()*/
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.S_un.S_addr = inet_addr(_DEF_FILE_SERVER_IP);
    server_addr.sin_port = htons(_DEF_FILE_SERVER_PROT);

    if (connect(sock, (const sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        closesocket(sock);
        return;
    }
    std::cout << "connect server[" << inet_ntoa(server_addr.sin_addr) << "] success." << std::endl;
    file::STRU_FILE_CONTENT_RQ rq;
    rq.method = rq.GET;
    strcpy_s(rq.filePath, remoteFilePath);
    int packSize = sizeof(rq);
    send(sock, (char*)&packSize, sizeof(int), 0); // 先发包大小
    send(sock, (char*)&rq, sizeof(rq), 0);        // 再发数据包
    QFileInfo fileInfo(remoteFilePath);
    QString writablePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir writableDir(writablePath);
    if (!writableDir.exists("user_file")) {
        writableDir.mkdir("user_file");
    }
    QDir personFileDir(writablePath + "/user_file");
    if (!personFileDir.exists(QString::number(uid))) {
        personFileDir.mkdir(QString::number(uid));
    }
    QString localFilePath =  writablePath + "/user_file/" + QString::number(uid) + "/" + fileInfo.fileName();
    qDebug() << "保存文件路径为:" << localFilePath;
    char buffer[8192];
    ssize_t bytesRead = 0;
    ssize_t totalBytesReceived = 0;
    QFile file(localFilePath);
    if (file.open(QIODevice::ReadWrite | QIODevice::Truncate)) { // 默认二进制打开
        while((bytesRead = recv(sock, buffer, 8192, 0)) > 0) {
            file.write(buffer, bytesRead);
            totalBytesReceived += bytesRead;
            if (totalBytesReceived >= fileSize) {
                qDebug() << __func__ << "读取文件完成";
                break;
            }
        }
        file.close();
    } else {
        qDebug() << __func__ << " open File Failed: " << file.errorString();
    }
    closesocket(sock);

    // 将下载的文件信息存入map和配置文件中
    m_mapFileMd5ToPath[fileMd5] = localFilePath;
    m_userIconSetting->setValue(QString("%1/filePath").arg(fileInfo.fileName()), localFilePath);
    m_userIconSetting->setValue(QString("%1/MD5").arg(fileInfo.fileName()), fileMd5);
}

void FileUtil::download(QByteArray &byteArray, QString& localAvatarPath, const char *remoteFilePath, int fileSize, const char *fileId, const char *fileMd5)
{
    if (m_mapFileMd5ToPath.find(fileMd5) != m_mapFileMd5ToPath.end()) {
        QString filePath = m_mapFileMd5ToPath[fileMd5];
        if (QFile::exists(filePath)) {
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly)) {
                byteArray = file.readAll();
                if(byteArray.size() == fileSize){
                    DEBUG << " filePath exist:" << filePath << " byteArray:" << byteArray.size();
                    localAvatarPath = filePath;
                    file.close();
                    return;
                }else{
                    file.close();
                    byteArray.clear();
                    if(QFile::remove(filePath)) {
                        DEBUG << "文件大小不一致，已删除文件: " << filePath;
                    } else {
                        DEBUG << "无法删除文件: " << filePath;
                    }
                }
            }
        }
    }

    QTcpSocket socket;
    socket.connectToHost(_DEF_FILE_SERVER_IP, _DEF_FILE_SERVER_PROT);

    if (!socket.waitForConnected()) {
        qDebug() << "连接服务器失败:" << socket.errorString();
        return;
    }

    file::STRU_FILE_CONTENT_RQ rq;
    rq.method = rq.GET;
    strcpy_s(rq.filePath, remoteFilePath);
    int packSize = sizeof(rq);
    socket.write(reinterpret_cast<const char*>(&packSize), sizeof(int)); // 先发包大小
    socket.write(reinterpret_cast<const char*>(&rq), sizeof(rq));        // 再发数据包
    if (!socket.waitForBytesWritten()) {
        DEBUG << "socket send failed : " << socket.errorString();
        return;
    }

    QFileInfo fileInfo(remoteFilePath);
    QString writablePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir writableDir(writablePath);
    if (!writableDir.exists("user_icons")) {
        writableDir.mkdir("user_icons");
    }
    QString localFilePath = writablePath + "/user_icons/" + fileInfo.fileName();
    qDebug() << "保存文件路径为:" << localFilePath;

    char buffer[8192];
    ssize_t totalBytesReceived = 0;
    QFile file(localFilePath);

    if (file.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        while (totalBytesReceived < fileSize) {
            if (!socket.waitForReadyRead()) {
                qDebug() << "等待数据超时:" << socket.errorString();
                break;
            }
            qint64 bytesRead = socket.read(buffer, sizeof(buffer));
            if (bytesRead > 0) {
                file.write(buffer, bytesRead);
                byteArray.append(buffer, bytesRead);
                totalBytesReceived += bytesRead;
            } else {
                qDebug() << "读取数据失败:" << socket.errorString();
                break;
            }
        }
        file.close();
    } else {
        qDebug() << __func__ << "打开文件失败:" << file.errorString();
    }
    socket.disconnectFromHost();
    qDebug() << __func__ << "() 下载文件完成, byteArray:" << byteArray.size();
    localAvatarPath = localFilePath;
    m_mapFileMd5ToPath[fileMd5] = localFilePath;
    m_userIconSetting->setValue(QString("%1/filePath").arg(fileInfo.fileName()), localFilePath);
    m_userIconSetting->setValue(QString("%1/MD5").arg(fileInfo.fileName()), fileMd5);
}

void FileUtil::upload(QString& remoteFilePath, QString& localFilePath, QString& fileId) {
    DEBUG << __func__;
    QFileInfo fileInfo(localFilePath);
    QTcpSocket socket;
    socket.connectToHost(_DEF_FILE_SERVER_IP, _DEF_FILE_SERVER_PROT);

    if (!socket.waitForConnected()) {
        qDebug() << "连接服务器失败:" << socket.errorString();
        return;
    }

    file::STRU_FILE_CONTENT_RQ rq;
    rq.method = rq.POST;
    rq.fileSize = fileInfo.size();
    strcpy_s(rq.filePath, (remoteFilePath + fileInfo.fileName()).toStdString().c_str());
    strcpy_s(rq.fileId, fileId.toStdString().c_str());
    int packSize = sizeof(rq);
    socket.write(reinterpret_cast<const char*>(&packSize), sizeof(int)); // 先发包大小
    socket.write(reinterpret_cast<const char*>(&rq), sizeof(rq));        // 再发数据包
    if (!socket.waitForBytesWritten()) {
        DEBUG << "socket send failed : " << socket.errorString();
        return;
    }
    // 发送文件块
    file::STRU_FILE_BLOCK_RQ block;
    int nReadLen = 0;  // 已读出的字节数
    QFile file(localFilePath);

    if (file.open(QIODevice::ReadOnly)) {
        do {
            nReadLen = file.read(block.fileContent, _DEF_FILE_CONTENT_SIZE);
            if (nReadLen > 0) {
                DEBUG << "发送文件" << fileInfo.fileName() << " fread() nReadLen:" << nReadLen;
                block.blockSize = nReadLen;
                strcpy_s(block.fileId, fileId.toStdString().c_str());
                int StruSize = sizeof(block);
                socket.write(reinterpret_cast<const char*>(&StruSize), sizeof(int));
                socket.write(reinterpret_cast<const char*>(&block), sizeof(block));
                if (!socket.waitForBytesWritten()) {
                    DEBUG << "socket send failed : " << socket.errorString();
                    return;
                }
            }
        } while (nReadLen > 0);
        file.close();
    } else {
        qDebug() << "无法打开文件:" << fileInfo.fileName();
    }
    DEBUG << "发送文件" << fileInfo.fileName() << "完成";
    socket.disconnectFromHost();
}
