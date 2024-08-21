#ifndef FILEINFO_H
#define FILEINFO_H

#include <string>
using namespace std;

#include "connection.h"
#include "connectionpool.h"

/*
+------------+--------------+------+-----+---------+-------+
| Field      | Type         | Null | Key | Default | Extra |
+------------+--------------+------+-----+---------+-------+
| file_id    | varchar(36)  | NO   | PRI | NULL    |       |
| file_name  | varchar(255) | NO   |     | NULL    |       |
| file_path  | text         | NO   |     | NULL    |       |
| file_size  | bigint       | NO   |     | NULL    |       |
| file_md5   | char(32)     | NO   |     | NULL    |       |
| file_state | varchar(50)  | NO   |     | PENDING |       |
+------------+--------------+------+-----+---------+-------+
*/

// User表的ORM类
class FileInfo
{
public:
    FileInfo(string fileid = "", string filename = "", string filepath = "", uint64_t file_size = 0, string filemd5 = "", string filestate = "PENDING"):
        fileid(fileid), filename(filename), filepath(filepath), file_size(file_size), filemd5(filemd5), filestate(filestate){}
    void setFileId(string fileid) {this->fileid = fileid;}
    void setFileName(string filename) { this->filename = filename; }
    void setFilePath(string filepath) { this->filepath = filepath; }
    void setFileSize(uint64_t file_size) { this->file_size = file_size; }
    void setFileMd5(string filemd5) { this->filemd5 = filemd5; }
    void setFileState(string filestate) { this->filestate = filestate; }

    string getFileId() { return this->fileid; }
    string getFileName() { return this->filename; }
    string getFilePath() { return this->filepath; }
    uint64_t getFileSize() { return this->file_size; }
    string getFileMd5() { return this->filemd5; }
    string getFileState() { return this->filestate; }

protected:
    string fileid;
    string filename;
    string filepath;
    uint64_t file_size;
    string filemd5;
    string filestate;
};

#endif