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
    bool startServer();

    void closeServer();
private:
    void setProtocolMap();

    void DestroyInstance();
public slots:

    void slot_DealData(unsigned long lSendIP, const char* buf, int nLen);

    void slot_HandleReportNetworkStatus(QString errdesc, QString status);

    void restoreFriendList(json jsonObject);

    void slot_LoginRs(json jsonObject);

    void slot_RegisterRs(json jsonObject);

    void slot_LoginCommit(QString username, QString password);

    void slot_RegisterCommit(QString username, QString tel, QString password);

    void slot_CloseLoginDialog();

    void parseFileInfo(const nlohmann::json& jsonObject, FileInfo& fileinfo);

    void slot_FriendInfoRs(json jsonObject);

    void slot_ChatRq(json jsonObject);

    // void slot_ChatRs(unsigned long lSendIP, const char* buf, int nLen);

    void slot_AddFriendRq(json jsonObject);

    // void slot_AddFriendRs(unsigned long lSendIP, const char* buf, int nLen);

    // void slot_DealFileInfoRq(unsigned long lSendIP, const char* buf, int nLen);
    //
    // void slot_DealFileInfoRs(unsigned long lSendIP, const char* buf, int nLen);
    //
    // void slot_DealFileBlockRq(unsigned long lSendIP, const char* buf, int nLen);
    //
    // void slot_DealFileBlockRs(unsigned long lSendIP, const char* buf, int nLen);

    void slot_dealGetUserInfoRs(json jsonObject);

    void slot_dealUpdateAvatarRs(json jsonObject);

public slots:
    void slot_SendChatMsg(int id, QString content);

    // void slot_SendFile(int id, QString filename, uint64_t filesize);

    void slot_GetFriendInfo(QString friendname);

    void slot_addFriendRequest(QString friendname, int friendId);

    void slot_FriendReqAccepted(int id);

    void slot_ChangeUserIcon();
private:

    // std::string GetFileName(const char* path);
    /**
     * @brief generateResourceMD5SumMap 将资源文件中的icon做MD5信息摘要
     */
    // void generateResourceMD5SumMap();

    void connectMainWindowSignals();
private:
    /// 协议映射表
    std::map<int, std::function<void(json jsonObject)> > m_deal_items;
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
