#ifndef FILEINFOMODEL_H
#define FILEINFOMODEL_H

#include "fileinfo.hpp"

// User表的数据操作类
class FileInfoModel {
public:

    bool insert(FileInfo &user);

    // 根据md5值查询文件记录
    FileInfo query(string filemd5);

    // 更新文件上传状态
    bool updateState(string fileid);
};

#endif