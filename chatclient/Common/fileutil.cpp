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

QSettings* FileUtil::m_userIconSetting = nullptr;
QSettings* FileUtil::m_userFileSetting = nullptr;
QMap<QString, QString> FileUtil::m_mapFileMd5ToPath;

FileUtil::FileUtil()
{
    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    m_userIconSetting = new QSettings("./UserIconConfig.ini", QSettings::IniFormat);
    m_userFileSetting = new QSettings("./UserFileConfig.ini", QSettings::IniFormat);
    m_userIconSetting->setIniCodec(codec);
    m_userFileSetting->setIniCodec(codec);
    m_userIconSetting->sync();
    m_userFileSetting->sync();
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
    // 判断本地是否有该头像
    // 1. 检查某个键是否存在
    if (m_mapFileMd5ToPath.find(fileMd5) != m_mapFileMd5ToPath.end()) {
        QString filePath = m_mapFileMd5ToPath[fileMd5];
        if (QFile::exists(filePath)) { // 文件存在
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly)) {
                byteArray = file.readAll();
                qDebug() << __func__ << "Map中存在该图片 filePath:" << filePath << " byteArray:" << byteArray.size();
                localAvatarPath = filePath;
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
    if (!writableDir.exists("user_icons")) {
        writableDir.mkdir("user_icons");
    }
    QString localFilePath =  writablePath + "/user_icons/" + fileInfo.fileName();
    qDebug() << "保存文件路径为:" << localFilePath;
    char buffer[8192];
    ssize_t bytesRead = 0;
    ssize_t totalBytesReceived = 0;
    QFile file(localFilePath);
    if (file.open(QIODevice::ReadWrite | QIODevice::Truncate)) { // 默认二进制打开
        while((bytesRead = recv(sock, buffer, 8192, 0)) > 0) {
            file.write(buffer, bytesRead);
            byteArray.append(buffer, bytesRead);
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
    qDebug() << __func__ << "() 下载文件完成, byteArray:" << byteArray.size();
    // 将下载的文件信息存入map和配置文件中
    localAvatarPath = localFilePath;
    m_mapFileMd5ToPath[fileMd5] = localFilePath;
    m_userIconSetting->setValue(QString("%1/filePath").arg(fileInfo.fileName()), localFilePath);
    m_userIconSetting->setValue(QString("%1/MD5").arg(fileInfo.fileName()), fileMd5);
}

void FileUtil::upload(QString& remoteFilePath, QString& localFilePath, QString& fileId) {
    qDebug() << __func__;
    QFileInfo fileInfo(localFilePath);
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
    rq.method = rq.POST;
    rq.fileSize = fileInfo.size();
    strcpy_s(rq.filePath, (remoteFilePath + fileInfo.fileName()).toStdString().c_str());
    strcpy_s(rq.fileId, fileId.toStdString().c_str());
    int packSize = sizeof(rq);
    send(sock, (char*)&packSize, sizeof(int), 0); // 先发包大小
    send(sock, (char*)&rq, sizeof(rq), 0);        // 再发数据包

    // 发送文件块
    file::STRU_FILE_BLOCK_RQ block;
//    uint64_t nPos = 0; // 已发送的位置
    int nReadLen = 0;  // 已读出的字节数
    QFile file(localFilePath);
    if (file.open(QIODevice::ReadOnly)) { // 默认二进制打开
        do {
            nReadLen = file.read(block.fileContent, _DEF_FILE_CONTENT_SIZE);
            qDebug() << "发送文件" << fileInfo.fileName() <<" fread() nReadLen:" << nReadLen;
            block.blockSize = nReadLen;
            strcpy_s(block.fileId, fileId.toStdString().c_str());
            int StruSize = sizeof(block);
            send(sock, (char*)&StruSize, sizeof(int), 0);
            send(sock, (char*)&block, sizeof(block), 0);
        } while(nReadLen > 0);
        file.close();
    } else {
        qDebug() << "无法打开文件:" << fileInfo.fileName();
    }
    qDebug() << "发送文件" << fileInfo.fileName() << " 完成";
    closesocket(sock);
}
