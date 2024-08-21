#include "fileinfomodel.hpp"
#include <iostream>
using namespace std;


bool FileInfoModel::insert(FileInfo &fileinfo)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into t_file(file_id, file_name, file_path, file_size, file_md5) values(UUID(),'%s', '%s', '%llu','%s')",
        fileinfo.getFileName().c_str(), 
        fileinfo.getFilePath().c_str(), 
        static_cast<unsigned long long>(fileinfo.getFileSize()), 
        fileinfo.getFileMd5().c_str());

    shared_ptr<Connection> sp_conn = ConnectionPool::getConnectionPool()->getConnection();
  
    if (sp_conn != nullptr && sp_conn->update(sql))
    {
        return true;
    }
    
    return false;
}


FileInfo FileInfoModel::query(string filemd5)
{
    char sql[1024] = {0};
    sprintf(sql, "select * from t_file where file_md5 = '%s'", filemd5.c_str());

    shared_ptr<Connection> sp_conn = ConnectionPool::getConnectionPool()->getConnection();
    if(sp_conn != nullptr){
        MYSQL_RES *res = sp_conn->query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                FileInfo fileinfo;
                fileinfo.setFileId(row[0]);
                fileinfo.setFileName(row[1]);
                fileinfo.setFilePath(row[2]);
                fileinfo.setFileSize(stoull(row[3]));
                fileinfo.setFileMd5(row[4]);
                fileinfo.setFileState(row[5]);
                mysql_free_result(res);
                return fileinfo;
            }
        }
    }

    return FileInfo();
}

bool FileInfoModel::updateState(string fileid)
{
    char sql[1024] = {0};
    sprintf(sql, "update t_file set file_state = '%s' where file_id = '%s'", "SUCCESS", fileid.c_str());

    shared_ptr<Connection> sp_conn = ConnectionPool::getConnectionPool()->getConnection();
    if (sp_conn != nullptr && sp_conn->update(sql))
    {
        return true;
    }

    return false;
}
