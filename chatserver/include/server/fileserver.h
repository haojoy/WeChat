#ifndef __FILESERVER_H__
#define __FILESERVER_H__

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <map>
#include <memory>
#include <string>
#include <cstdio>
using namespace std;
using namespace placeholders;
using namespace muduo;
using namespace muduo::net;

enum PackType : uint32_t {
    FILE_CONTENT_RQ = 1000,
    FILE_CONTENT_RS,
    FILE_BLOCK_RQ
};

#ifndef _MAX_FILE_PATH_SIZE
#define _MAX_FILE_PATH_SIZE     (512)
#endif

#ifndef _DEF_FILE_CONTENT_SIZE
#define _DEF_FILE_CONTENT_SIZE  (8192)
#endif

#define _DEF_FILE_POS_PREFIX ("/home/mycode/chatserver")
// 文件传输协议

// 1. Client<->Server<->Client
/**
 * @brief 文件信息
*/
/// MD5字符数组大小
#ifndef _MD5_STR_SIZE
#define _MD5_STR_SIZE (33)
#endif
#define _FILEID_STR_SIZE (37)
struct FileInfo
{
    FileInfo() : nPos(0), fileSize(0), pFile(nullptr) {
        memset(fileId  , 0, _MAX_FILE_PATH_SIZE);
        memset(fileName, 0, _MAX_FILE_PATH_SIZE);
        memset(filePath, 0, _MAX_FILE_PATH_SIZE);
        memset(md5     , 0, _MD5_STR_SIZE      );
    }
    /// @brief 文件唯一id
    char fileId[_MAX_FILE_PATH_SIZE];
    /// @brief 文件名
    char fileName[_MAX_FILE_PATH_SIZE];
    /// @brief 文件所在路径
    char filePath[_MAX_FILE_PATH_SIZE];
    /// @brief 文件MD5值
    char md5[_MD5_STR_SIZE];
    /// @brief 文件已经接受的字节数
    uint64_t nPos;
    /// @brief 文件大小
    uint64_t fileSize;
    /// @brief 文件指针
    FILE* pFile;
};

// 1. Client<->Server
struct STRU_FILE_CONTENT_RQ {
    enum Method {
        GET,
        POST
    };
    STRU_FILE_CONTENT_RQ()
        : type(FILE_CONTENT_RQ)
        , method(GET)
        , fileSize(0) {
        memset(filePath, 0, _MAX_FILE_PATH_SIZE);
        memset(fileId, 0, _FILEID_STR_SIZE);
    }
    /// @brief 
    PackType type;
    /// @brief
    Method method;
    /// @brief
    int fileSize;
    /// @brief 
    char filePath[_MAX_FILE_PATH_SIZE];
    /// @brief
    char fileId[_FILEID_STR_SIZE];
};

struct STRU_FILE_BLOCK_RQ {
    STRU_FILE_BLOCK_RQ() : type(FILE_BLOCK_RQ), blockSize(0) {
        memset(fileId, 0, _FILEID_STR_SIZE);
        memset(fileContent, 0, _DEF_FILE_CONTENT_SIZE);
    }
    /// @brief 数据包类型: FILE_BLOCK_RQ
    PackType type;
    /// @brief 文件唯一id
    char fileId[_FILEID_STR_SIZE];
    /// @brief 文件块内容
    char fileContent[_DEF_FILE_CONTENT_SIZE];
    /// @brief 文件块大小
    uint64_t blockSize;
};

class FileServer {
public:
    FileServer(EventLoop* loop, const InetAddress& listenAddr);

    void start();  // 启动服务器

    void dealData(const TcpConnectionPtr& conn, const char* buf, int buflen);
private:
    void onConnection(const TcpConnectionPtr& conn);  // 处理新连接
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time);  // 处理客户端消息

    void dealFileContentRq(const TcpConnectionPtr& conn, const char* buf, int buflen);  // 处理文件内容请求
    void dealFileBlockRq(const TcpConnectionPtr& conn, const char* buf, int buflen);  // 处理文件块请求

    bool sendFile(const TcpConnectionPtr& conn, const string& filePath);  // 发送文件

private:
    TcpServer server_;
    map<int, function<void(const TcpConnectionPtr&, const char*, int)>> m_deal_items;  // 协议头与处理函数的映射
    map<string, FileInfo*> m_mapFileIdToFileInfo;  // 文件Id与文件信息的映射
};

#endif // __FILESERVER_H__