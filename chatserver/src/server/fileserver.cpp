#include "fileserver.h"
#include <muduo/base/Logging.h>
#include <fcntl.h>
#include <sys/sendfile.h>

FileServer::FileServer(EventLoop* loop, const InetAddress& listenAddr)
    : server_(loop, listenAddr, "FileServer") {
    server_.setConnectionCallback(std::bind(&FileServer::onConnection, this, _1));
    server_.setMessageCallback(std::bind(&FileServer::onMessage, this, _1, _2, _3));
    
    // 设置协议处理函数映射
    m_deal_items[FILE_CONTENT_RQ] = std::bind(&FileServer::dealFileContentRq, this,_1, _2, _3);
    m_deal_items[FILE_BLOCK_RQ] = std::bind(&FileServer::dealFileBlockRq, this, _1, _2, _3);
}

void FileServer::start() {
    server_.start();
    LOG_INFO << "[FileServer] started and listening on " << server_.ipPort();
}

void FileServer::onConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {
        LOG_INFO << "[FileServer] New connection from " << conn->peerAddress().toIpPort();
    } else {
        LOG_INFO << "[FileServer] Connection from " << conn->peerAddress().toIpPort() << " closed";
    }
}

void FileServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf, muduo::Timestamp time) {
    while (buf->readableBytes() >= sizeof(int)) {  // 首先读取包大小
        const char* data = buf->peek();
        int nPackSize = *reinterpret_cast<const int*>(data);
        if (buf->readableBytes() >= sizeof(int) + nPackSize) {  // 检查缓冲区是否有完整的数据包
            buf->retrieve(sizeof(int));  // 消费掉包大小
            const char* message = buf->peek();
            dealData(conn, message, nPackSize);
            buf->retrieve(nPackSize);  // 消费掉消息内容
        } else {
            break;  // 数据不完整，等待更多数据到达
        }
    }
}

void FileServer::dealData(const TcpConnectionPtr& conn, const char* buf, int buflen) {
    int header_type = *reinterpret_cast<const int*>(buf);
    auto it = m_deal_items.find(header_type);
    if (it != m_deal_items.end()) {
        it->second(conn, buf, buflen);  // 调用对应的处理函数
    } else {
        LOG_ERROR << "Unknown message type: " << header_type;
    }
}

void FileServer::dealFileContentRq(const TcpConnectionPtr& conn, const char* buf, int buflen) {
    const STRU_FILE_CONTENT_RQ* rq = reinterpret_cast<const STRU_FILE_CONTENT_RQ*>(buf);
    if (rq->method == STRU_FILE_CONTENT_RQ::GET) {
        sendFile(conn, _DEF_FILE_POS_PREFIX + std::string(rq->filePath));
    } else if (rq->method == STRU_FILE_CONTENT_RQ::POST) {
        FileInfo* fileInfo = new FileInfo;
        fileInfo->fileSize = rq->fileSize;
        fileInfo->nPos = 0;
        strcpy(fileInfo->fileId, rq->fileId);
        fileInfo->pFile = fopen((_DEF_FILE_POS_PREFIX + std::string(rq->filePath)).c_str(), "w+");
        if (fileInfo->pFile == nullptr) {
            LOG_ERROR << "Failed to open file: " << _DEF_FILE_POS_PREFIX + std::string(rq->filePath);
            return;
        }
        if (m_mapFileIdToFileInfo.find(fileInfo->fileId) == m_mapFileIdToFileInfo.end()) {
            m_mapFileIdToFileInfo[fileInfo->fileId] = fileInfo;
            LOG_INFO << __func__ << " file open success :" << _DEF_FILE_POS_PREFIX + std::string(rq->filePath);
        }
    }
}

void FileServer::dealFileBlockRq(const TcpConnectionPtr& conn, const char* buf, int buflen) {
    const STRU_FILE_BLOCK_RQ* rq = reinterpret_cast<const STRU_FILE_BLOCK_RQ*>(buf);
    auto it = m_mapFileIdToFileInfo.find(rq->fileId);
    if (it == m_mapFileIdToFileInfo.end()) {
        LOG_ERROR << "No file info found for file ID: " << rq->fileId;
        return;
    }
    FileInfo *fileinfo = m_mapFileIdToFileInfo[rq->fileId];
    auto fd              = fileinfo->pFile;
    int nResult    = fwrite(rq->fileContent, sizeof(char), rq->blockSize, fd);
    fileinfo->nPos += nResult;
    if (fileinfo->nPos >= fileinfo->fileSize) { // 已经收到所有文件块
        LOG_INFO << __func__ << "file has been receive complete" << fileinfo->fileName;
        fclose(fd);             // 关闭文件指针
        m_mapFileIdToFileInfo.erase(rq->fileId);
        delete fileinfo;
        fileinfo = nullptr;
    }
}

bool FileServer::sendFile(const TcpConnectionPtr& conn, const std::string& filePath) {
    LOG_INFO << __func__ << " filePath" << filePath;
     // 打开文件
    int fd = open(filePath.c_str(), O_RDONLY);
    if (fd == -1)
    {
        LOG_ERROR << "Failed to open file: " << filePath << ", error: " << strerror(errno);
        return false;
    }

    // 获取文件大小
    off_t fileSize = lseek(fd, 0, SEEK_END);
    if (fileSize == -1)
    {
        LOG_ERROR << "Failed to get file size: " << filePath << ", error: " << strerror(errno);
        close(fd);
        return false;
    }
    lseek(fd, 0, SEEK_SET);

    // 读取文件内容到 Buffer
    const size_t chunkSize = 64 * 1024; // 64KB
    char buffer[chunkSize];
    ssize_t bytesRead;
    ssize_t remaining = fileSize;

    while (remaining > 0)
    {
        bytesRead = read(fd, buffer, std::min(chunkSize, static_cast<size_t>(remaining)));
        if (bytesRead <= 0)
        {
            LOG_ERROR << "Failed to read from file: " << filePath << ", error: " << strerror(errno);
            close(fd);
            return false;
        }

        // 将读取的数据发送到客户端
        conn->send(buffer, bytesRead);
        remaining -= bytesRead;
        LOG_INFO << "filesize:" << fileSize  << " remaining:" << remaining<< "Bytes not send";
    }

    // 关闭文件
    close(fd);

    LOG_INFO << "File sent successfully: " << filePath;
    return true;
}
