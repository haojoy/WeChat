#ifndef KERNEL_H
#define KERNEL_H

#include <functional>
#include <map>
#include <list>
#include <string>
#include <QByteArray>
#include <QSettings>
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
#include <QFile>
#include <QEvent>
#include <QMouseEvent>
#include <QDir>
#include <QDirIterator>
#include <QCryptographicHash>
#include <QTextCodec>
#include <QStandardPaths>

#include "Net/INetMediator.h"
#include "Net/packdef.h"
#include "logindialog.h"
#include "mainwindow.h"
#include "Views/userinfoshowdialog.h"
#include "Common/fileutil.h"

class Kernel : public QObject
{
    Q_OBJECT
public:
    Kernel();
    ~Kernel();
public:
    static QString& GetName() { return m_username; }
    static QString& GetAvatarUrl() { return m_avatarUrl; }
public:
    /**
     * @brief 开启服务器
     * @return 成功返回true; 失败返回false
     */
    bool startServer();
    /**
     * @brief 关闭服务器
     */
    void closeServer();
private:
    /**
     * @brief 设置函数映射
     */
    void setProtocolMap();
    /**
     * @brief DestroyInstance 回收堆区分配的空间
     */
    void DestroyInstance();
public slots:
    /**
     * @brief 处理数据
     * @param lSendIP
     * @param buf
     * @param nLen
     */
    void slot_DealData(unsigned long lSendIP, const char* buf, int nLen);
    /**
     * @brief 处理登录回复
     * @param lSendIP
     * @param buf
     * @param nLen
     */
    void slot_LoginRs(unsigned long lSendIP, const char* buf, int nLen);
    /**
     * @brief 处理注册回复
     * @param lSendIP
     * @param buf
     * @param nLen
     */
    void slot_RegisterRs(unsigned long lSendIP, const char* buf, int nLen);
    /**
     * @brief slot_LoginCommit 登录提交事件
     * @param username 用户名
     * @param password 密码
     */
    void slot_LoginCommit(QString username, QString password);
    /**
     * @brief slot_RegisterCommit 注册提交事件
     * @param username 用户名
     * @param tel 电话号码
     * @param password 密码
     */
    void slot_RegisterCommit(QString username, QString tel, QString password);
    /**
     * @brief slot_FriendInfoRq 获取好友信息请求
     * @param lSendIP
     * @param buf
     * @param nLen
     */
    void slot_FriendInfoRs(unsigned long lSendIP, const char* buf, int nLen);
    /**
     * @brief slot_ChatRq 好友发来聊天请求
     * @param lSendIP
     * @param buf
     * @param nLen
     */
    void slot_ChatRq(unsigned long lSendIP, const char* buf, int nLen);
    /**
     * @brief slot_ChatRs 自己发出的聊天内容的回复
     * @param lSendIP
     * @param buf
     * @param nLen
     */
    // void slot_ChatRs(unsigned long lSendIP, const char* buf, int nLen);
    /**
     * @brief slot_AddFriendRq 接收到别人申请添加好友的请求
     * @param lSendIP
     * @param buf
     * @param nLen
     */
    void slot_AddFriendRq(unsigned long lSendIP, const char* buf, int nLen);
    /**
     * @brief slot_AddFriendRs 接收到别人同意添加好友的回复
     * @param lSendIP
     * @param buf
     * @param nLen
     */
    // void slot_AddFriendRs(unsigned long lSendIP, const char* buf, int nLen);

    // void slot_DealFileInfoRq(unsigned long lSendIP, const char* buf, int nLen);
    //
    // void slot_DealFileInfoRs(unsigned long lSendIP, const char* buf, int nLen);
    //
    // void slot_DealFileBlockRq(unsigned long lSendIP, const char* buf, int nLen);
    //
    // void slot_DealFileBlockRs(unsigned long lSendIP, const char* buf, int nLen);

    void slot_dealGetUserInfoRs(unsigned long lSendIP, const char* buf, int nLen);

    void slot_dealUpdateAvatarRs(unsigned long lSendIP, const char* buf, int nLen);

public slots:
    /**
     * @brief slot_SendChatMsg 聊天信息发送按钮点击事件
     * @param id
     * @param content
     */
    void slot_SendChatMsg(int id, QString content);
    /**
     * @brief SIG_SendFile 点击发送文件按钮向kernel发送文件信息信号
     * @param id    要发送给的好友的id
     * @param filename 文件名，包括路径
     * @param filesize 文件大小
     */
    // void slot_SendFile(int id, QString filename, uint64_t filesize);
    /**
     * @brief slot_GetFriendInfo 点击获取欲添加好友的信息
     * @param friendname
     */
    void slot_GetFriendInfo(QString friendname);
    /**
     * @brief slot_addFriendRequest 点击添加好友到通讯录
     * @param friendname
     */
    void slot_addFriendRequest(QString friendname, int friendId);
    /**
     * @brief slot_FriendReqAccepted 同意添加好友
     * @param id
     */
    void slot_FriendReqAccepted(int id);
    /**
     * @brief slot_ChangeUserIcon 处理更换头像操作
     */
    void slot_ChangeUserIcon();
private:
    /**
     * @brief GetFileName 从文件路径中获取文件名
     * @param path 文件绝对路径
     * @return
     */
    // std::string GetFileName(const char* path);
    /**
     * @brief generateResourceMD5SumMap 将资源文件中的icon做MD5信息摘要
     */
    // void generateResourceMD5SumMap();
private:
    /// 协议映射表
    std::map<int, std::function<void(unsigned long, const char*, int)> > m_deal_items;
    /// 文件id与文件信息映射表
    /// std::map<std::string, FileInfo*> m_mapFileIdToFileInfo;
    /// 好友id与好友头像的映射表
    /// std::map<int, QPixmap*> m_mapFriendIdToIcon;
    /// 资源头像的路径与MD5信息摘要映射
    /// QMap<QString, QByteArray> m_mapIconUrlToMd5;
    /// 网络中介者
    net::INetMediator* m_pClient;
    /// 登录UI对象指针
    LoginDialog* m_loginDialog;
    /// 主界面UI对象指针
    MainWindow* m_mainWnd;
    // 个人信息
    /// 用户唯一id
    int m_uuid;
    /// 用户名称
    static QString m_username;
    /// 头像URL
    static QString m_avatarUrl;
    /// 头像MD5
    static QString m_avatarMd5;
    /// 用户状态
    int m_state;
    /// 用户个性签名
    QString m_feeling;
    /// 文件传输系统
    FileUtil* m_fileUtil;
};

#endif // KERNEL_H
