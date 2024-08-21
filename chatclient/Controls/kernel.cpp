#include "kernel.h"
#include "Net/TcpClientMediator.h"
#include "Net/INet.h"
#include "Models/friend.h"
#include "Models/message.h"
#include "Common/fileutil.h"
#include <QTimer>
#include <QApplication>
#include <QtCore/qstring.h>
#include "Common/logger.h"
QString Kernel::m_username;
QString Kernel::m_avatarUrl = "";
QString Kernel::m_avatarMd5;

Kernel::Kernel() : m_uuid(0), m_state(0)
{
    setProtocolMap();               // 初始化协议映射表
    // generateResourceMD5SumMap();    // 初始化资源头像MD5信息摘要
    m_fileUtil = new FileUtil;
    m_pClient = new net::TcpClientMediator;
    connect(m_pClient, SIGNAL(SIG_ReadyData(ulong,const char*,int)), this, SLOT(slot_DealData(ulong,const char*,int)));
    connect(m_pClient, SIGNAL(SIG_ReportNetworkStatus(QString,QString)), this, SLOT(slot_HandleReportNetworkStatus(QString, QString)));

    // 注册登录界面注册
    m_loginDialog = new LoginDialog;
    connect(m_loginDialog, SIGNAL(SIG_LoginCommit(QString,QString)), this, SLOT(slot_LoginCommit(QString, QString)));
    connect(m_loginDialog, SIGNAL(SIG_RegisterCommit(QString,QString,QString)), this, SLOT(slot_RegisterCommit(QString,QString,QString)));
    connect(m_loginDialog, SIGNAL(SIG_CloseLoginDialog()), this, SLOT(slot_CloseLoginDialog()));
    m_loginDialog->showNormal();

    if (!startServer()) {
        DEBUG << "连接服务器失败!";
        //m_loginDialog->showErrorTips("连接服务器失败!");
    }

    m_mainWnd = new MainWindow(nullptr, m_uuid); // m_uuid还未被正确设定值
    // 发送聊天信息
    connect(m_mainWnd, SIGNAL(sendChatMessage(int,QString)), this, SLOT(slot_SendChatMsg(int, QString)));
    // 发送聊天文件
    //connect(m_mainWnd, SIGNAL(sendChatFile(int, QString, uint64_t)), this, SLOT(slot_SendFile(int, QString, uint64_t)));
    // 发送获取好友信息
    connect(m_mainWnd, SIGNAL(sendGetFriendInfo(QString)), this, SLOT(slot_GetFriendInfo(QString)));
    // 发送添加好友请求
    connect(m_mainWnd, SIGNAL(sendAddFriendRequest(QString,int)), this, SLOT(slot_addFriendRequest(QString, int)));
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
    auto call = std::bind(&Kernel::func, this, std::placeholders::_1); \
        m_deal_items.insert({ str, call });}


XX(LOGIN_MSG_ACK, slot_LoginRs);
XX(REG_MSG_ACK, slot_RegisterRs);
XX(REFRESH_FRIEND_LIST, slot_FriendInfoRs);
XX(GET_FRIEND_INFO_RSP, slot_dealGetUserInfoRs);
XX(ADD_FRIEND_REQ, slot_AddFriendRq);
XX(ONE_CHAT_MSG, slot_ChatRq);
XX(SET_AVATAR_RS, slot_dealUpdateAvatarRs);
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
    try {
        json jsonObject = json::parse(string(buf, nLen));
        int msgid = jsonObject["msgid"].get<int>();
        m_deal_items[msgid](jsonObject);

    }catch (const std::exception& e) {
        DEBUG << "Error processing data: " << e.what();
    }
}

void Kernel::slot_HandleReportNetworkStatus(QString errdesc, QString errstatus)
{
    m_loginDialog->showErrorTips(errdesc, errstatus);
}

void Kernel::restoreFriendList(json jsonObject) {
    vector<User> currentUserFriendList;

    if (jsonObject.contains("friends"))
    {
        // 初始化
        currentUserFriendList.clear();

        vector<string> vec = jsonObject["friends"];
        for (string &str : vec)
        {
            json js = json::parse(str);
            User user;
            user.setId(js["userid"].get<int>());
            //user.setAvatarId(js["userid"].get<int>());
            user.setName(QString::fromStdString(js["name"]));
            user.setState(QString::fromStdString(js["state"]));
            currentUserFriendList.push_back(user);

            QByteArray avatarByteArray;
            QString avatarPath;
            FileInfo fileinfo;

            if (js.contains("avatarinfo")) {
                string avatarinfo = js["avatarinfo"];
                json fileInfo = json::parse(avatarinfo);
                parseFileInfo(fileInfo, fileinfo);
            }
            FileUtil::download(avatarByteArray, avatarPath, fileinfo.filePath, fileinfo.fileSize, fileinfo.fileId, fileinfo.md5);
            QList<Message> messages1;
            m_mainWnd->getChatListWidget()->AddItem(new Friend(user.getId(), user.getName(), avatarPath, true, "20/11/28", 0, messages1));
        }
    }
}

void Kernel::slot_LoginRs(json jsonObject) {
    int loginstatus = jsonObject["errno"].get<int>();

    switch(loginstatus) {
    case LOGIN_OK:
    {
        restoreFriendList(jsonObject);
        QByteArray avatarByteArray;
        QString avatarPath;
        FileInfo fileinfo;

        if (jsonObject.contains("avatarinfo")) {
            string avatarinfo = jsonObject["avatarinfo"];
            json fileInfo = json::parse(avatarinfo);
            parseFileInfo(fileInfo, fileinfo);
        }
        DEBUG << fileinfo.filePath;
        FileUtil::download(avatarByteArray, avatarPath, fileinfo.filePath, fileinfo.fileSize, fileinfo.fileId, fileinfo.md5);
        m_mainWnd->setAvatar(avatarByteArray);
        m_loginDialog->hide();
        m_mainWnd->show(); // 显示主窗口
        m_uuid = jsonObject["userid"].get<int>();
        m_avatarUrl = avatarPath;
        m_username = QString::fromStdString(jsonObject["name"].get<std::string>());
    }
    break;
    case LOGIN_ONLINE:
        QMessageBox::about(this->m_loginDialog, "提示", "用户已在线");
        break;
    case LOGIN_INVALID_USERORPWD:
        QMessageBox::about(this->m_loginDialog, "错误", "用户名或密码错误");
        break;
    default:
        DEBUG << "invalid loginstatus: " <<loginstatus;
    }
    m_loginDialog->showErrorTips("");
}

void Kernel::slot_RegisterRs(json jsonObject) {

    int regstatus = jsonObject["errno"].get<int>();
    switch(regstatus) {
    case REG_OK:
        QMessageBox::about(this->m_loginDialog, "提示", "注册成功");
        break;
    case REG_USER_EXIST:
        QMessageBox::about(this->m_loginDialog, "提示", "用户已存在");
        break;
    case REG_ERR:
        QMessageBox::about(this->m_loginDialog, "错误", "注册失败");
        break;
    default:
        DEBUG << "regstatus: " <<regstatus;
    }
}

void Kernel::slot_LoginCommit(QString username, QString password) {
    m_loginDialog->showErrorTips("正在登录...", Normal);
    if(!startServer()){
        m_loginDialog->showErrorTips("网络异常，请稍后重试", Error);
    }
    std::string strUserName = username.toStdString();
    std::string strPassWord = password.toStdString();
    json js;
    js["msgid"] = LOGIN_MSG;
    js["name"] = strUserName;
    js["password"] = strPassWord;
    string request = js.dump();
    m_pClient->SendData(0, request.c_str(), request.size());
}

void Kernel::slot_RegisterCommit(QString username, QString tel, QString password) {
    if(!startServer()){
        m_loginDialog->showErrorTips("注册失败,网络异常，请稍后重试", Error);
    }
    std::string strUserName = username.toStdString();
    std::string strTel = tel.toStdString();
    std::string strPassWord = password.toStdString();

    json js;
    js["msgid"] = REG_MSG;
    js["name"] = strUserName;
    js["tel"] = strTel;
    js["password"] = strPassWord;
    string request = js.dump();
    m_pClient->SendData(0, request.c_str(), request.size());


}
void Kernel::slot_CloseLoginDialog(){
    QTimer::singleShot(0, QCoreApplication::instance(), &QCoreApplication::quit); // 退出事件循环
}

void Kernel::parseFileInfo(const nlohmann::json& jsonObject, FileInfo& fileinfo) {

    if (jsonObject.contains("file_id")) {
        std::string fileIdStr = jsonObject["file_id"];
        strncpy(fileinfo.fileId, fileIdStr.c_str(), _MAX_FILE_PATH_SIZE - 1);
        fileinfo.fileId[_MAX_FILE_PATH_SIZE - 1] = '\0';
    }
    if (jsonObject.contains("file_name")) {
        std::string fileNameStr = jsonObject["file_name"];
        strncpy(fileinfo.fileName, fileNameStr.c_str(), _MAX_FILE_PATH_SIZE - 1);
        fileinfo.fileName[_MAX_FILE_PATH_SIZE - 1] = '\0';
    }
    if (jsonObject.contains("file_path")) {
        std::string filePathStr = jsonObject["file_path"];
        strncpy(fileinfo.filePath, filePathStr.c_str(), _MAX_FILE_PATH_SIZE - 1);
        fileinfo.filePath[_MAX_FILE_PATH_SIZE - 1] = '\0';
    }
    if (jsonObject.contains("file_md5")) {
        std::string md5Str = jsonObject["file_md5"];
        strncpy(fileinfo.md5, md5Str.c_str(), _MD5_STR_SIZE - 1);
        fileinfo.md5[_MD5_STR_SIZE - 1] = '\0';
    }
    if (jsonObject.contains("file_size")) {
        fileinfo.fileSize = jsonObject["file_size"].get<uint64_t>();
    }

}
void Kernel::slot_FriendInfoRs(json jsonObject) {

    int userid = jsonObject["userid"].get<int>();
    QString username = QString::fromStdString(jsonObject["username"]);
    // int state = jsonObject["state"].get<int>();
    DEBUG << "m_uuid: "  << m_uuid << "userid: " << userid;


    FileInfo fileinfo;

    if (jsonObject.contains("avatarinfo")) {
        string avatarinfo = jsonObject["avatarinfo"];
        json fileInfo = json::parse(avatarinfo);
        parseFileInfo(fileInfo, fileinfo);
    }
    QByteArray avatarByteArray;
    QString avatarPath;
    FileUtil::download(avatarByteArray, avatarPath, fileinfo.filePath, fileinfo.fileSize, fileinfo.fileId, fileinfo.md5);
    if(m_uuid == userid) { // 该用户信息是自己的
        m_username = username;
        //m_feeling = info->feeling;
        //m_state = state;
        m_avatarUrl = avatarPath;
        m_mainWnd->setAvatar(avatarByteArray);
        return;
    }
    if (!m_mainWnd->getChatListWidget()->isContainsKey(userid)) { // 用户信息是好友的, 且还没添加到map中
        QList<Message> messages1;
        m_mainWnd->getChatListWidget()->AddItem(new Friend(userid, username, avatarPath, true, "20/11/28", 0, messages1));
    }
}

void Kernel::slot_SendChatMsg(int id, QString content) {
    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["senderid"] = m_uuid;
    js["receiverid"] = id;
    js["msginfo"] = content.toStdString();
    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    js["createtime"] = currentTime.toStdString();
    string request = js.dump();
    m_pClient->SendData(0, request.c_str(), request.size());
}

void Kernel::slot_ChatRq(json jsonObject) {
    int senderid = jsonObject["senderid"].get<int>();
    string msgcont = jsonObject["msginfo"];
    string createtime = jsonObject["createtime"];
    auto item = m_mainWnd->getChatListWidget()->getItemById(senderid);
    if(item) {
        Message message(QString::fromStdString(msgcont), QString::fromStdString(createtime), Sender);
        item->updateContent(message);
    }

}

void Kernel::slot_AddFriendRq(json jsonObject)
{
    int senderid = jsonObject["senderid"].get<int>();
    string sendername = jsonObject["sendername"];
    DEBUG << "好友id:" << senderid << "名称：" << sendername.c_str() << "请求添加好友";
    m_mainWnd->getContactWidget()->setItem(senderid); // rq.senderId是请求添加好友的用户id
    m_mainWnd->getContactWidget()->getItem(senderid)->setId(senderid);
    m_mainWnd->getContactWidget()->getItem(senderid)->setName(QString::fromStdString(sendername));

    FileInfo fileinfo;
    if (jsonObject.contains("avatarinfo")) {
        string avatarinfo = jsonObject["avatarinfo"];
        json fileInfo = json::parse(avatarinfo);
        parseFileInfo(fileInfo, fileinfo);
    }
    QByteArray avatarByteArray;
    QString avatarPath;
    FileUtil::download(avatarByteArray, avatarPath, fileinfo.filePath, fileinfo.fileSize, fileinfo.fileId, fileinfo.md5);
    QPixmap pixmap;
    pixmap.loadFromData(avatarByteArray);
    if (pixmap.isNull()) {
        qDebug() << "头像数据错误";
        // return;
    }
    m_mainWnd->getContactWidget()->getItem(senderid)->setIcon(pixmap);
    m_mainWnd->getContactWidget()->addNewFriend(senderid);

}

void Kernel::slot_GetFriendInfo(QString username) // 根据用户想要添加的好友获取好友信息
{
    std::string strUserName = username.toStdString();
    json js;
    js["msgid"] = GET_FRIEND_INFO_REQ;
    js["name"] = strUserName;
    string request = js.dump();
    m_pClient->SendData(0, request.c_str(), request.size());
}

void Kernel::slot_dealGetUserInfoRs(json jsonObject)
{
    int getinfostatus = jsonObject["errno"].get<int>();
    switch(getinfostatus) {
    case GET_FRIEND_INFO_NO_THIS_USER:
        m_mainWnd->getContactList()->modifySearchStackedWidgetIndex();
        break;
    case GET_FRIEND_INFO_SUCCESS:
    {
        m_mainWnd->getUserInfoDialog()->setUserName(QString::fromStdString(jsonObject["name"]));
        m_mainWnd->getUserInfoDialog()->setUserId(jsonObject["userid"].get<int>());

        FileInfo fileinfo;
        if (jsonObject.contains("avatarinfo")) {
            string avatarinfo = jsonObject["avatarinfo"];
            json fileInfo = json::parse(avatarinfo);
            parseFileInfo(fileInfo, fileinfo);
        }
        QByteArray avatarByteArray;
        QString avatarPath;
        FileUtil::download(avatarByteArray, avatarPath, fileinfo.filePath, fileinfo.fileSize, fileinfo.fileId, fileinfo.md5);

        m_mainWnd->getUserInfoDialog()->setUserAvatar(avatarByteArray);
        QPoint windowPos = m_mainWnd->mapToGlobal(QPoint(0, 0));
        m_mainWnd->getUserInfoDialog()->move(windowPos.x()+305, windowPos.y()+65);
        m_mainWnd->getUserInfoDialog()->show();
    }
    break;
    default:
        break;
    }
}

void Kernel::slot_dealUpdateAvatarRs(json jsonObject)
{
    int uploadstatus = jsonObject["errno"].get<int>();
    string avatarId, uploadpath;
    if (jsonObject.contains("avatarid")) {
        avatarId = jsonObject["avatarid"];

    }
    if(jsonObject.contains("filepath")){
        uploadpath = jsonObject["filepath"];
    }

    DEBUG <<  "result: " <<uploadstatus << "path: " <<  uploadpath.c_str() << "avatarId: " <<  avatarId.c_str();
    if(uploadstatus == NOTNEEDUPLOAD) {
        DEBUG << "服务器有该头像无需上传";
        m_mainWnd->setAvatarId(QString::fromUtf8(avatarId.c_str()));
    }else if (uploadstatus== NEEDUPLOAD) { // 需要上传头像
        DEBUG << "需要上传头像";
        QString remotePath = QString::fromUtf8(uploadpath.c_str());
        QString fileId = QString::fromUtf8(avatarId.c_str());
        FileUtil::upload(remotePath, m_avatarUrl, fileId);
        m_mainWnd->setAvatarId(QString::fromUtf8(avatarId.c_str()));
        // 告知服务器头像已经上传完成
        QFileInfo fileInfo(m_avatarUrl);

        json js;
        js["msgid"] = SET_AVATAR_COMPLETE_NOTIFY;
        js["senderId"] = m_uuid;
        js["avatarid"] = avatarId;
        string request = js.dump();
        m_pClient->SendData(0, request.c_str(), request.size());
    }
}

void Kernel::slot_addFriendRequest(QString friendname, int friendId)
{
    json js;
    js["msgid"] = ADD_FRIEND_REQ;
    js["senderid"] = m_uuid;
    js["sendername"] = m_username.toStdString();
    js["receiverid"] = friendId;
    js["receivername"] = friendname.toStdString();
    string request = js.dump();
    m_pClient->SendData(0, request.c_str(), request.size());
}

void Kernel::slot_FriendReqAccepted(int id) // 发送添加好友回复
{
    // TODO 在好友列表显示该好友

    // QList<Message> messages1;
    //m_mainWnd->getChatListWidget()->AddItem(new Friend(id, username, ":/images/icon/2.png", true, "20/11/28", 0, messages1));
    // 告知好友同意

    json js;
    js["msgid"] = ADD_FRIEND_RSP;
    js["senderid"] = m_uuid;
    js["receiverid"] = id;
    js["result"] = ADD_FRIEND_ACCEPT;
    string request = js.dump();
    m_pClient->SendData(0, request.c_str(), request.size());
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
    DEBUG << targetFilePath;
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


    json js;
    js["msgid"] = SET_AVATAR_RQ;
    js["senderId"] = m_uuid;
    js["filesize"] = newInfo.size();
    js["filename"] = newInfo.fileName().toStdString();
    js["filemd5"] = fileMd5.toStdString();

    string request = js.dump();
    m_pClient->SendData(0, request.c_str(), request.size());

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
