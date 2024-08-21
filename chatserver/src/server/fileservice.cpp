#include "fileservice.h"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <vector>
#include <fstream>
#include <fcntl.h>
using namespace std;
using namespace muduo;

// 获取单例对象的接口函数
FileService *FileService::instance()
{
    static FileService service;
    return &service;
}

// 注册消息以及对应的Handler回调操作
FileService::FileService()
{
    _msgHandlerMap.insert({FILE_CONTENT_RQ, std::bind(&FileService::dealFileContentRq, this, _1, _2, _3)});
    _msgHandlerMap.insert({FILE_BLOCK_RQ, std::bind(&FileService::dealFileBlockRq, this, _1, _2, _3)});
}


MsgHandler FileService::getHandler(int msgid)
{
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {
        return [=](const TcpConnectionPtr &conn, const char* buf, Timestamp) {
            LOG_ERROR << "FileService msgid:" << msgid << " can not find handler!";
        };
    }
    else
    {
        return _msgHandlerMap[msgid];
    }
}

void FileService::dealFileContentRq(const TcpConnectionPtr& conn, const char* buf, Timestamp time) {
    const STRU_FILE_CONTENT_RQ* rq = reinterpret_cast<const STRU_FILE_CONTENT_RQ*>(buf);
    if (rq->method == STRU_FILE_CONTENT_RQ::GET) {
        sendFile(conn, _DEF_FILE_POS_PREFIX + std::string(rq->filePath));
    } else if (rq->method == STRU_FILE_CONTENT_RQ::POST) {
        FileInfo_* fileInfo = new FileInfo_;
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

void FileService::dealFileBlockRq(const TcpConnectionPtr& conn, const char* buf, Timestamp time) {
    const STRU_FILE_BLOCK_RQ* rq = reinterpret_cast<const STRU_FILE_BLOCK_RQ*>(buf);
    auto it = m_mapFileIdToFileInfo.find(rq->fileId);
    if (it == m_mapFileIdToFileInfo.end()) {
        LOG_ERROR << "No file info found for file ID: " << rq->fileId;
        return;
    }
    FileInfo_ *fileinfo = m_mapFileIdToFileInfo[rq->fileId];
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

bool FileService::sendFile(const TcpConnectionPtr& conn, const std::string& filePath) {
    LOG_INFO << __func__ << " filePath: " << filePath;
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
    const size_t chunkSize = 8 * 1024; // 8KB
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

        conn->send(buffer, bytesRead);
        remaining -= bytesRead;
        usleep(10000); // 10 毫秒延迟
    }

    // 关闭文件
    close(fd);

    LOG_INFO << "File sent successfully: " << filePath;
    return true;
}

