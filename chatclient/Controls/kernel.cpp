#include "kernel.h"
#include "Net/TcpClientMediator.h"
#include "Net/INet.h"
#include "Models/friend.h"
#include "Models/message.h"
#include "Common/fileutil.h"
#include <QTimer>
#include <QApplication>
#include <QtCore/qstring.h>

QString Kernel::m_username;
QString Kernel::m_avatarUrl = ":/images/icon/5.png";
QString Kernel::m_avatarMd5;

Kernel::Kernel() : m_uuid(0), m_state(0)
{
    setProtocolMap();               // 初始化协议映射表
    // generateResourceMD5SumMap();    // 初始化资源头像MD5信息摘要
    m_fileUtil = new FileUtil;
    m_pClient = new net::TcpClientMediator;
    connect(m_pClient, SIGNAL(SIG_ReadyData(ulong,const char*,int)), this, SLOT(slot_DealData(ulong,const char*,int)));
    if (!startServer()) {
        qDebug() << "连接服务器失败!";
    }
    // 注册登录界面注册
    m_loginDialog = new LoginDialog;
    connect(m_loginDialog, SIGNAL(SIG_LoginCommit(QString, QString)), this, SLOT(slot_LoginCommit(QString, QString)));
    connect(m_loginDialog, SIGNAL(SIG_RegisterCommit(QString,QString,QString)), this, SLOT(slot_RegisterCommit(QString,QString,QString)));

    if (m_loginDialog->exec() != QDialog::Accepted) {
        m_loginDialog->hide();
        QTimer::singleShot(0, QCoreApplication::instance(), &QCoreApplication::quit); // 退出事件循环
    }
    m_mainWnd = new MainWindow(nullptr, m_uuid); // m_uuid还未被正确设定值
    // 发送聊天信息
    connect(m_mainWnd, SIGNAL(sendChatMessage(int, QString)), this, SLOT(slot_SendChatMsg(int, QString)));
    // 发送聊天文件
    //connect(m_mainWnd, SIGNAL(sendChatFile(int, QString, uint64_t)), this, SLOT(slot_SendFile(int, QString, uint64_t)));
    // 发送获取好友信息
    connect(m_mainWnd, SIGNAL(sendGetFriendInfo(QString)), this, SLOT(slot_GetFriendInfo(QString)));
    // 发送添加好友请求
    connect(m_mainWnd, SIGNAL(sendAddFriendRequest(QString, int)), this, SLOT(slot_addFriendRequest(QString, int)));
    // 发送接受添加回复
    connect(m_mainWnd, SIGNAL(sendFriendReqAccepted(int)), this, SLOT(slot_FriendReqAccepted(int)));
    // 发送修改头像信息
    connect(m_mainWnd, SIGNAL(SIG_ChangeUserIcon()), this, SLOT(slot_ChangeUserIcon()));
}

Kernel::~Kernel() {
    DestroyInstance();
}

bool Kernel::startServer() {
    if (!m_pClient->OpenNet()) return false;
    return true;
}

void Kernel::closeServer() {
    if (m_pClient)
        m_pClient->CloseNet();
}

void Kernel::setProtocolMap() {
#define XX(str, func) {\
    auto call = std::bind(&Kernel::func, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3); \
        m_deal_items.insert({ str, call });}

XX(_DEF_PACK_LOGIN_RS, slot_LoginRs);
XX(_DEF_PACK_REGISTER_RS, slot_RegisterRs);
XX(_DEF_PACK_FRIEND_INFO, slot_FriendInfoRs);
XX(_DEF_PROTOCOL_GETUSERINFO_RS, slot_dealGetUserInfoRs);
XX(_DEF_PACK_ADDFRIEND_RQ, slot_AddFriendRq);
XX(_DEF_PACK_CHAT_RQ, slot_ChatRq);
XX(UPDATE_AVATAR_RS, slot_dealUpdateAvatarRs);
#undef XX
}

void Kernel::DestroyInstance() {
    if (m_loginDialog) {
        m_loginDialog->hide();
        delete m_loginDialog;
        m_loginDialog = nullptr;
    }

    if (m_pClient) {
        m_pClient->CloseNet();
        delete m_pClient;
        m_pClient = nullptr;
    }

    if (m_fileUtil) {
        delete m_fileUtil;
        m_fileUtil = nullptr;
    }

    if (m_mainWnd) {
        delete m_mainWnd;
        m_mainWnd = nullptr;
    }
}

void Kernel::slot_DealData(unsigned long lSendIP, const char* buf, int nLen) {
    int header_type = *(int*)buf;
    qDebug() << __func__ << " : " << header_type;
    if (header_type >= _DEF_PROTOCOL_BASE && header_type <= _DEF_PROTOCOL_BASE + _DEF_PROTOCOL_COUNT)
        m_deal_items[header_type](lSendIP, buf, nLen);
}

void Kernel::slot_LoginRs(unsigned long lSendIP, const char* buf, int nLen) {
    Q_UNUSED(lSendIP);
    Q_UNUSED(nLen);
    STRU_LOGIN_RS* rs = (STRU_LOGIN_RS *)buf;
    qDebug() << __func__ << "  " << rs->result << "  " << rs->userid;
    switch(rs->result) {
    case user_not_exist:
        QMessageBox::about(this->m_loginDialog, "提示", "登录失败，用户不存在");
        break;
    case password_error:
        QMessageBox::about( this->m_loginDialog , "提示", "登录失败,密码错误");
        break;
    case login_wait:
        m_uuid = rs->userid;
        break;
    case login_success:
        {
            qDebug() << "login_success";
            m_loginDialog->hide();

            m_mainWnd->show(); // 显示主窗口
            m_uuid = rs->userid;
        }
        break;
    }
}

void Kernel::slot_RegisterRs(unsigned long lSendIP, const char* buf, int nLen) {
    Q_UNUSED(lSendIP);
    Q_UNUSED(nLen);
    STRU_REGISTER_RS* rs = (STRU_REGISTER_RS*)buf;
    switch(rs->result) {
    case user_is_exist:
        QMessageBox::about(this->m_loginDialog, "提示", "注册失败, 用户已存在");
        break;
    case register_success:
        QMessageBox::about(this->m_loginDialog, "提示", "注册成功");
        break;
    }
}

void Kernel::slot_LoginCommit(QString username, QString password) {
    std::string strUserName = username.toStdString();
    std::string strPassWord = password.toStdString();
    STRU_LOGIN_RQ rq;
    strcpy_s(rq.username, strUserName.c_str());
    strcpy_s(rq.password, strPassWord.c_str());
    // qstrncpy(rq.username, strUserName.c_str(), sizeof(rq.username));
    // qstrncpy(rq.password, strPassWord.c_str(), sizeof(rq.password));
    m_pClient->SendData(0, (char*)&rq, sizeof(rq));
}

void Kernel::slot_RegisterCommit(QString username, QString tel, QString password) {
    std::string strUserName = username.toStdString();
    std::string strTel = tel.toStdString();
    std::string strPassWord = password.toStdString();
    STRU_REGISTER_RQ rq;
    strcpy_s(rq.username, strUserName.c_str());
    strcpy_s(rq.tel, strTel.c_str());
    strcpy_s(rq.password, strPassWord.c_str());

    m_pClient->SendData(0, (char*)&rq, sizeof(rq));
}

void Kernel::slot_FriendInfoRs(unsigned long lSendIP, const char* buf, int nLen) {
    Q_UNUSED(lSendIP);
    Q_UNUSED(nLen);
    STRU_FRIEND_INFO* info = (STRU_FRIEND_INFO*)buf;
    QString username = QString::fromStdString(info->username);
    QByteArray avatarByteArray;
    QString avatarPath;
    FileUtil::download(avatarByteArray, avatarPath, info->avatarInfo.filePath, info->avatarInfo.fileSize, info->avatarInfo.fileId, info->avatarInfo.md5);
    if(m_uuid == info->uuid) { // 该用户信息是自己的
        m_username = info->username;
        m_feeling = info->feeling;
        m_state = info->state;
        m_avatarUrl = avatarPath;
        m_mainWnd->setAvatar(avatarByteArray);
        return;
    }
    if (!m_mainWnd->getChatListWidget()->isContainsKey(info->uuid)) { // 用户信息是好友的, 且还没添加到map中
        QList<Message> messages1;
        m_mainWnd->getChatListWidget()->AddItem(new Friend(info->uuid, username, avatarPath, true, "20/11/28", 0, messages1));
    }
}

void Kernel::slot_ChatRq(unsigned long lSendIP, const char* buf, int nLen) {
    Q_UNUSED(lSendIP);
    Q_UNUSED(nLen);
    STRU_CHAT_RQ* rq = (STRU_CHAT_RQ*)buf; // 对方好友发来的聊天数据包(userid是好友的id)
    auto item = m_mainWnd->getChatListWidget()->getItemById(rq->userid);
    if(item) {
        Message message(QString::fromStdString(rq->content), QString::fromStdString(rq->createTime), Sender);
        item->updateContent(message);
    }
}

void Kernel::slot_AddFriendRq(unsigned long lSendIP, const char *buf, int nLen)
{
    // 收到他人的添加好友声请, 在好友列表的RightBar添加好友信息
    Q_UNUSED(lSendIP);
    Q_UNUSED(nLen);
    STRU_ADD_FRIEND_RQ* rq = (STRU_ADD_FRIEND_RQ*)buf;
    qDebug() << __func__ << "好友id:" << rq->senderId << "名称：" << rq->senderName << "请求添加好友";
    m_mainWnd->getContactWidget()->setItem(rq->senderId); // rq.senderId是请求添加好友的用户id
    m_mainWnd->getContactWidget()->getItem(rq->senderId)->setId(rq->senderId);
    m_mainWnd->getContactWidget()->getItem(rq->senderId)->setName(QString::fromStdString(rq->senderName));

    // 显示sender头像
    QByteArray byteArray;
    QString avatarPath;
    FileUtil::download(byteArray, avatarPath, rq->senderAvatarInfo.filePath, rq->senderAvatarInfo.fileSize, rq->senderAvatarInfo.fileId, rq->senderAvatarInfo.md5);
    QPixmap pixmap;
    pixmap.loadFromData(byteArray);
    if (pixmap.isNull()) {
        qDebug() << "头像数据错误";
        return;
    }
    m_mainWnd->getContactWidget()->getItem(rq->senderId)->setIcon(pixmap);
    m_mainWnd->getContactWidget()->addNewFriend(rq->senderId);
}



void Kernel::slot_dealGetUserInfoRs(unsigned long lSendIP, const char *buf, int nLen)
{
    Q_UNUSED(lSendIP);
    Q_UNUSED(nLen);
    STRU_GET_USERINFO_RS* rs = (STRU_GET_USERINFO_RS*)buf;

    switch (rs->result) {
    case STRU_GET_USERINFO_RS::NO_THIS_USER:
        // 显示QStackedWidget的Index为1
        qDebug() << rs->result << " " << "no_this_user";
        m_mainWnd->getContactList()->modifySearchStackedWidgetIndex();
        break;
    case STRU_GET_USERINFO_RS::GETINFO_SUCCESS:
        // 该人存在, RigBar显示信息
        qDebug() << rs->result << " " << "getinfo_success";
        m_mainWnd->getUserInfoDialog()->setUserName(QString::fromStdString(rs->userName));
        m_mainWnd->getUserInfoDialog()->setUserId(rs->userId);
        // download头像
        QByteArray byteArray;
        QString temp;
        FileUtil::download(byteArray, temp, rs->filePath, rs->fileSize, rs->fileId, rs->fileMd5);

        m_mainWnd->getUserInfoDialog()->setUserAvatar(byteArray);
        QPoint windowPos = m_mainWnd->mapToGlobal(QPoint(0, 0));
        m_mainWnd->getUserInfoDialog()->move(windowPos.x()+305, windowPos.y()+65);
        m_mainWnd->getUserInfoDialog()->show();
        break;
    }
}

void Kernel::slot_dealUpdateAvatarRs(unsigned long lSendIP, const char *buf, int nLen)
{
    Q_UNUSED(lSendIP);
    Q_UNUSED(nLen);
    qDebug() << __func__;
    STRU_UPDATE_AVATAR_RS* rs = (STRU_UPDATE_AVATAR_RS*)buf;
    if (rs->result == rs->NOTNEEDUPLOAD) { // 服务器有该头像无需上传
        qDebug() << __func__ << "服务器有该头像无需上传";
        m_mainWnd->setAvatarId(QString::fromUtf8(rs->avatarId));
    } else if (rs->result == rs->NEEDUPLOAD) { // 需要上传头像
        qDebug() << __func__ << "需要上传头像";
        QString remotePath = rs->uploadPath;
        QString fileId = rs->avatarId;
        FileUtil::upload(remotePath, m_avatarUrl, fileId);
        m_mainWnd->setAvatarId(QString::fromUtf8(rs->avatarId));
        // 告知服务器头像已经上传完成
        QFileInfo fileInfo(m_avatarUrl);
        STRU_UPDATE_AVATAR_COMPLETE_NOTIFY notify;
        notify.senderId = m_uuid;
        notify.result = notify.UPLOADSUCCESS;
        notify.fileSize = fileInfo.size();
        strcpy_s(notify.fileName, fileInfo.fileName().toStdString().c_str());
        strcpy_s(notify.avatarId, rs->avatarId);
        strcpy_s(notify.fileMd5, m_avatarMd5.toStdString().c_str());
        m_pClient->SendData(0, (char*)&notify, sizeof(notify));
    }
}

void Kernel::slot_SendChatMsg(int id, QString content) {
    STRU_CHAT_RQ rq;
    rq.userid = m_uuid;
    rq.friendid = id;
    std::string strContent = content.toStdString();
    strcpy_s(rq.content,strContent.c_str());
    // qDebug() << __func__ << rq.content;
    m_pClient->SendData(0, (char*)&rq, sizeof(rq));
}

void Kernel::slot_GetFriendInfo(QString userName) // 根据用户想要添加的好友获取好友信息
{
    STRU_GET_USERINFO_RQ rq;
    strcpy_s(rq.userName, userName.toStdString().c_str());
    strcpy_s(rq.senderName, m_username.toStdString().c_str());
    rq.senderId = m_uuid;
    m_pClient->SendData(0, (char*)&rq, sizeof(rq));
}

void Kernel::slot_addFriendRequest(QString friendname, int friendId)
{
    STRU_ADD_FRIEND_RQ rq;
    strcpy_s(rq.receiverName, friendname.toStdString().c_str());
    strcpy_s(rq.senderName, m_username.toStdString().c_str());
    rq.senderId = m_uuid;
    rq.receiverId = friendId;
    /// 服务器添加头像信息
    m_pClient->SendData(0, (char*)&rq, sizeof(rq));
}

void Kernel::slot_FriendReqAccepted(int id) // 发送添加好友回复
{
    // TODO 在好友列表显示该好友

    // QList<Message> messages1;
    //m_mainWnd->getChatListWidget()->AddItem(new Friend(id, username, ":/images/icon/2.png", true, "20/11/28", 0, messages1));
    // 告知好友同意
    STRU_ADD_FRIEND_RS rs;
    rs.receiverId = id;
    rs.senderId = m_uuid;
    strcpy(rs.senderName, m_username.toStdString().c_str());
    rs.result = rs.ADD_SUCCESS;
    m_pClient->SendData(0, (char*)&rs, sizeof(rs));
}

void Kernel::slot_ChangeUserIcon()
{
    // 1. 从系统选择头像
    QString filePath = QFileDialog::getOpenFileName(m_mainWnd, "选择头像", QDir::homePath(), QObject::tr("Images (*.png *.jpg);;All Files (*)"));
    if (filePath.isEmpty()) return;  // 未选择头像
    // 2. 判断该头像类型是否合规(.png, .jpg)
    if (!filePath.endsWith(".png") && !filePath.endsWith(".jpg")) { // 选择的文件不合规
        QMessageBox::about(m_mainWnd, "提示", "文件类型必须为png或jpg");
        return;
    }
    // 3. 将该图片复制到程序数据存储目录
    QFileInfo fileInfo(filePath);
    QString writablePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir writableDir(writablePath);
    if (!writableDir.exists("user_icons")) {
        writableDir.mkdir("user_icons");
    }
    QString targetFilePath = writablePath + "/user_icons/" + fileInfo.fileName();
    // 3.1 判断项目的程序数据存储目录是否已经存在该图片
    if (QFile::exists(targetFilePath)) { // 目标目录下已经存在该图片
        int i = 1;
        while(true) {
            QString newTargetFile = QString("%1(%2).%3").arg(fileInfo.baseName()).arg(i++).arg(fileInfo.completeSuffix());
            targetFilePath = writablePath + "/user_icons/" + newTargetFile;
            if (!QFile::exists(targetFilePath)) break; // 经过修改, 该路径下不存在同名函数了, 该文件名可用
        }
    }
    QFile::copy(filePath, targetFilePath); // 复制新文件
    // 3.2 维护图片与MD5信息摘要的Config
    QString fileMd5;
    FileUtil::insertMD5IntoConfig(targetFilePath, fileMd5);
    // 4. 修改用户头像
    m_avatarUrl = targetFilePath;
    m_mainWnd->setIcon(m_avatarUrl);
    qDebug() << __func__ << "m_avatarUrl:" << m_avatarUrl;
    // 5. 通知服务器更改用户信息
    QFileInfo newInfo(targetFilePath);
    STRU_UPDATE_AVATAR_RQ rq;
    rq.senderId = m_uuid;
    rq.fileSize = newInfo.size();
    strcpy_s(rq.fileName, newInfo.fileName().toStdString().c_str());
    strcpy_s(rq.oldAvatarId, m_mainWnd->getAvatarId().toStdString().c_str());
    strcpy_s(rq.fileMd5, fileMd5.toStdString().c_str());
    m_pClient->SendData(0, (char*)&rq, sizeof(rq));
}

//std::string Kernel::GetFileName(const char* path) {
//    int nlen = strlen(path);
//    if (nlen < 1) {
//        return std::string();
//    }
//    for (int i = nlen - 1; i >= 0; i --) {
//        if (path[i] == '\\' || path[i] == '/') {
//            return &path[i+1];
//        }
//    }
//    return std::string();
//}

//void Kernel::generateResourceMD5SumMap()
//{
//    // 存储到配置文件中
//    QString rootPath = QStringLiteral(":/images/icon/"); // 获取资源文件中的根目录路径
//    QStringList images; // 存储所有图片文件的路径
//    if (QDir(rootPath).exists()) { // 检查是否存在根目录
//        QDirIterator it(rootPath, QDirIterator::Subdirectories);
//        while (it.hasNext()) {
//            QString path = it.next();
//            if (QFileInfo(path).isFile() && QFileInfo(path).suffix() == "png") {
//                images.append(path);
//            }
//        }
//    }
//    foreach (QString imagePath, images) {
//        QFile file(imagePath);
//        if (file.open(QIODevice::ReadOnly)) {
//            QCryptographicHash hash(QCryptographicHash::Md5);
//            if (hash.addData(&file)) {
//                QByteArray md5Sum(hash.result());
//                m_mapIconUrlToMd5.insert(imagePath, md5Sum.toHex()); // 记录MD5信息摘要
//            }
//            file.close();
//        }
//    }
//}

// void Kernel::slot_AddFriendRs(unsigned long lSendIP, const char *buf, int nLen)
// {
//    // 在好友列表显示该好友
// }

// void Kernel::slot_DealFileInfoRq(unsigned long lSendIP, const char* buf, int nLen) {
//    Q_UNUSED(lSendIP);
//    Q_UNUSED(nLen);
//    qDebug() << __func__;
//    STRU_FILE_INFO_RQ* rq = (STRU_FILE_INFO_RQ*)buf;
//    STRU_FILE_INFO_RS rs;
//    strcpy_s(rs.szFileId, rq->szFileId); // 文件id的文件名字段应该是现在保存的文件名
//    rs.uuid = m_uuid;
//    rs.friendid = rq->uuid;
//    char text[1024] = "";
//    sprintf(text, "%d发来了%s, 是否接受?", rq->friendid, rq->szFileName);
//    QMessageBox::StandardButton button = QMessageBox::question(m_mainWnd, "好友发来文件", QString::fromStdString(text), QMessageBox::StandardButtons(QMessageBox::Save|QMessageBox::Cancel), QMessageBox::NoButton);
//    if (button == QMessageBox::Save) {
//        QString fileName = QFileDialog::getSaveFileName(m_mainWnd, "选择保存文件路径", ".", tr("文本文件(*.txt);;所有文件 (*.*)"));
//        qDebug() << "getSaveFileName() 选中的文件名:  " << fileName;
//        // 将文件信息存储到map中
//        FileInfo* info = new FileInfo;
//        info->fileSize = rq->nFileSize;
//        info->nPos = 0;
//        strcpy_s(info->szFileId, rq->szFileId);
//        strcpy_s(info->szFileName, rq->szFileName);
//        strcpy_s(info->szFilePath, fileName.toStdString().c_str());
//        if (fopen_s(&info->pFile, info->szFilePath, "wb") != 0) {
//            qDebug() << "打开文件失败 fopen_s(&info->pFile, info->szFilePath, 'wb')";
//            return;
//        }
//        if (!info->pFile) {
//            qDebug() << "pFile是空指针";
//            return;
//        }
//        if (m_mapFileIdToFileInfo.find(info->szFileId) == m_mapFileIdToFileInfo.end()) {
//            qDebug() << "接收端同意接受文件, 保存文件信息, info->szFileId " << info->szFileId << " info->szFileName: " << info->szFileName << " info->szFilePath:" << info->szFilePath;
//            m_mapFileIdToFileInfo[info->szFileId] = info;
//        }
//        rs.nResult = _file_accept;
//    } else if (button == QMessageBox::Cancel) {
//        rs.nResult = _file_refuse;
//    }
//    m_pClient->SendData(lSendIP, (char*)&rs, sizeof(rs));
//    return;
// }
// void Kernel::slot_DealFileInfoRs(unsigned long lSendIP, const char* buf, int nLen) {
//    Q_UNUSED(lSendIP);
//    Q_UNUSED(nLen);
//    qDebug() << __func__;
//    STRU_FILE_INFO_RS* rs = (STRU_FILE_INFO_RS*)buf;
//    if (rs->nResult == _file_accept) {
//        qDebug() << "好友同意接受";
//        STRU_FILE_BLOCK_RQ rq;
//        uint64_t nPos = 0;
//        int nReadLen = 0;
//        if (m_mapFileIdToFileInfo.find(rs->szFileId) != m_mapFileIdToFileInfo.end()) {
//            qDebug() << "好友同意接受, 接受的文件id为" << rs->szFileId;
//            FileInfo* info = m_mapFileIdToFileInfo[rs->szFileId];
//            while(true) {
//                nReadLen = fread(rq.szFileContent, sizeof(char), _DEF_FILE_CONTENT_SIZE, info->pFile);
//                qDebug() << "发送文件 fread() nReadLen:" << nReadLen;
//                rq.nBlockSize = nReadLen;
//                strcpy_s(rq.szFileId, rs->szFileId);
//                rq.friendid = rs->uuid;
//                rq.uuid = m_uuid;
//                m_pClient->SendData(lSendIP, (char*)&rq, sizeof(rq));
//                nPos += nReadLen;
//                if (nPos >= info->nFileSize || nReadLen < _DEF_FILE_CONTENT_SIZE) {
//                    fclose(info->pFile);
//                    m_mapFileIdToFileInfo.erase(rs->szFileId);
//                    delete info;
//                    info = nullptr;
//                    break;
//                }
//            }
//        }
//    } else {
//        qDebug() << "好友拒绝接受";
//        QMessageBox::about(m_mainWnd, "提示", "对方拒绝接受");
//        // 将文件信息删掉
//        if(m_mapFileIdToFileInfo.find(rs->szFileId) != m_mapFileIdToFileInfo.end()) {
//            FileInfo* info = m_mapFileIdToFileInfo[rs->szFileId];
//            fclose(info->pFile);
//            m_mapFileIdToFileInfo.erase(rs->szFileId);
//            delete info;
//            info = nullptr;
//        }
//    }
// }
// void Kernel::slot_DealFileBlockRq(unsigned long lSendIP, const char* buf, int nLen) {
//    Q_UNUSED(lSendIP);
//    Q_UNUSED(nLen);
//    qDebug() << __func__;
//    STRU_FILE_BLOCK_RQ* rq = (STRU_FILE_BLOCK_RQ*)buf;
//    if(m_mapFileIdToFileInfo.find(rq->szFileId) == m_mapFileIdToFileInfo.end()) {
//        qDebug() << "接收到文件块请求，但没有保存文件信息无法接受";
//        return;
//    }
//    FileInfo* info = m_mapFileIdToFileInfo[rq->szFileId];
//    qDebug() << __func__ << "接受到的文件内容是" << rq->szFileContent;
//    qDebug() << "上次同意接受文件保存的文件信息 info->szFileId " << info->szFileId << " info->szFileName: " << info->szFileName << " info->szFilePath:" << info->szFilePath;
//    int nResult = fwrite(rq->szFileContent, sizeof(char), rq->nBlockSize, info->pFile);
//    info->nPos += nResult;
//    qDebug() << "文件现在已经接受的大小为: " << info->nPos;
//    if (info->nPos >= info->nFileSize) {
//        // 已经传输完毕
//        qDebug() << "文件" << info->szFileName << "传输完毕";
//        // 关闭文件指针
//        fclose(info->pFile);
//        // 从map中删掉文件信息
//        m_mapFileIdToFileInfo.erase(rq->szFileId);
//        delete info;
//        info = nullptr;
//        // 发送接受成功回复包
//        STRU_FILE_BLOCK_RS rs;
//        strcpy_s(rs.szFileId, rq->szFileId);
//        rs.nResult = _file_block_recv_success;
//        rs.uuid = m_uuid;
//        rs.friendid = rq->uuid;
//        m_pClient->SendData(lSendIP, (char*)&rs, sizeof(rs));
//        QMessageBox::about(m_mainWnd, "提示", "已经成功接受该文件");
//    }
// }
// void Kernel::slot_DealFileBlockRs(unsigned long lSendIP, const char* buf, int nLen) {
//    Q_UNUSED(lSendIP);
//    Q_UNUSED(nLen);
//    STRU_FILE_BLOCK_RS *rs = (STRU_FILE_BLOCK_RS*)buf;
//    if (rs->nResult == _file_block_recv_success) {
//        // 对端已经成功接受
//        if(m_mapFileIdToFileInfo.find(rs->szFileId) != m_mapFileIdToFileInfo.end()) {
//            qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!这行不应该出现!!!!!!!!!!!!!!";
//            FileInfo* info = m_mapFileIdToFileInfo[rs->szFileId];
//            fclose(info->pFile);
//            m_mapFileIdToFileInfo.erase(rs->szFileId);
//            delete info;
//            info = nullptr;
//        }
//        QMessageBox::about(m_mainWnd, "提示", "对方已经成功接受");
//    }
// }

// void Kernel::slot_SendFile(int id, QString filename, uint64_t filesize) {
//    qDebug() << __func__ << " id: " << id << " filename:" << filename << " filesize:" << filesize;
//    STRU_FILE_INFO_RQ rq;
//    std::string strfilename = GetFileName(filename.toStdString().c_str());
//    strcpy_s(rq.szFileName, strfilename.c_str());
//    std::string strtime = QTime::currentTime().toString("hh_mm_ss_zzz").toStdString();
//    sprintf(rq.szFileId, "%s_%s", strfilename.c_str(), strtime.c_str());
//    rq.uuid = m_uuid;
//    rq.friendid = id;
//    rq.nFileSize = filesize;
//    // 将文件信息存储到FileInfo结构体中
//    FileInfo *info = new FileInfo;
//    info->nPos = 0;
//    info->nFileSize = filesize;
//    strcpy_s(info->szFileId, rq.szFileId);
//    strcpy_s(info->szFileName, strfilename.c_str());
//    strcpy_s(info->szFilePath, filename.toStdString().c_str());
//    if (fopen_s(&info->pFile, info->szFilePath, "rb") != 0) {
//        qDebug() << "主动发送文件端打开文件失败 errno=" << errno << " reason = " << strerror(errno);
//        return;
//    }
//    if (!info->pFile) {
//        qDebug() << "主动发送文件端info->pFile为空";
//    }
//    if (m_mapFileIdToFileInfo.find(info->szFileId) == m_mapFileIdToFileInfo.end()) {
//        m_mapFileIdToFileInfo[info->szFileId] = info;
//    }
//    m_pClient->SendData(0, (char*)&rq, sizeof(rq));
// }

//void Kernel::slot_ChatRs(unsigned long lSendIP, const char* buf, int nLen) {
//    Q_UNUSED(lSendIP);
//    Q_UNUSED(nLen);
//}
