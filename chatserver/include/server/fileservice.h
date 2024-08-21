#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <mutex>
using namespace std;
using namespace muduo;
using namespace muduo::net;

#include "json.hpp"
using json = nlohmann::json;

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
struct FileInfo_
{
    FileInfo_() : nPos(0), fileSize(0), pFile(nullptr) {
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

#define _DEF_FILE_POS_PREFIX ("/home/mycode/chatserver")

// 表示处理消息的事件回调方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr &conn, const char* buf, Timestamp time)>;

class FileService
{
public:
    // 获取单例对象的接口函数
    static FileService *instance();
    MsgHandler getHandler(int action);

    // 处理文件内容请求
    void dealFileContentRq(const TcpConnectionPtr& conn, const char* buf, Timestamp time); 
    // 处理文件块请求 
    void dealFileBlockRq(const TcpConnectionPtr& conn, const char* buf, Timestamp time); 
    // 发送文件 
    bool sendFile(const TcpConnectionPtr& conn, const string& filePath);  
private:
    FileService();
    // 存储消息id和其对应的业务处理方法
    unordered_map<int, MsgHandler> _msgHandlerMap;
    unordered_map<string, FileInfo_*> m_mapFileIdToFileInfo;  // 文件Id与文件信息的映射
};

#endif