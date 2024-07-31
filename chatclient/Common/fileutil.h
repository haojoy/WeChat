#ifndef FILEUTIL_H
#define FILEUTIL_H
#include <QSettings>
#include <QPixmap>
class FileUtil
{
public:
    FileUtil();
    static void insertMD5IntoConfig(QString&& filePath);
    static void insertMD5IntoConfig(QString& filePath, QString& fileMd5);
    static void download(const char* remoteFilePath, int fileSize, const char* fileId, const char* fileMd5, int uid);
    static void download(QByteArray& byteArray, QString& localFilePath, const char* remoteFilePath, int fileSize, const char* fileId, const char* fileMd5);
    static void upload(QString& remoteFilePath, QString& localFilePath, QString& fileId);
private:
    /// 用户头像配置文件
    static QSettings* m_userIconSetting;
    /// 用户聊天文件信息配置文件
    static QSettings* m_userFileSetting;
    /// 文件MD5值与文件路径的映射
    static QMap<QString, QString> m_mapFileMd5ToPath;
};

#endif // FILEUTIL_H
