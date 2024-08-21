#include "connection.h"
#include <iostream>
#include <muduo/base/Logging.h>
using namespace std;

const string ip_ = "127.0.0.1";
const string username_ = "haojoy";
const string password_ = "Haojoy@123";
const int port_ = 3306;
const string dbname_ = "wechat";

int Connection::createDBCnt_ = 0;
Connection::Connection():_conn(nullptr)
{
	// 初始化数据库连接
	_conn = mysql_init(nullptr);
	if (!_conn) {
		LOG_ERROR << "MySQL initialization failed";
		return;
	}
	
	if (!createDBTables()) {
		LOG_ERROR << "Failed to create database and tables";
	}
}

Connection::~Connection()
{
	// 释放数据库连接资源
	if (_conn != nullptr)
		mysql_close(_conn);
		_conn = nullptr;
}

bool Connection::createDBTables() {
	if(createDBCnt_++ != 0){
		return true;
	}

	if (mysql_real_connect(_conn, ip_.c_str(), username_.c_str(), password_.c_str(), nullptr, port_, nullptr, 0) == nullptr) {
        LOG_ERROR << "MySQL connection error: " << mysql_error(_conn);
        return false;
    }

	string queryStr = "CREATE DATABASE IF NOT EXISTS `" + dbname_ + "`";
	if (mysql_query(_conn, queryStr.c_str()) != 0) {
		LOG_ERROR << "MySQL createDatabase error: " << mysql_error(_conn);
		return false;
	}

	queryStr = "USE `" + dbname_ + "`";
	if (mysql_query(_conn, queryStr.c_str()) != 0) {
		LOG_ERROR << "MySQL useDatabase error: " << mysql_error(_conn);
		return false;
	}

	string sql_tuser = "CREATE TABLE IF NOT EXISTS `t_user` (\
		`userid` int NOT NULL AUTO_INCREMENT PRIMARY KEY, \
		`avatar_id` VARCHAR(36) DEFAULT NULL, \
		`username` VARCHAR(64) DEFAULT NULL, \
		`password` VARCHAR(64) DEFAULT NULL, \
		`tel` VARCHAR(15) DEFAULT NULL, \
		`state` enum('online','offline') CHARACTER SET latin1 DEFAULT 'offline' \
	)ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
	if (mysql_query(_conn, sql_tuser.c_str()) != 0) {
		LOG_ERROR << "MySQL createTable  t_user error: " << mysql_error(_conn);
		return false;
	}

	string sql_tfile = "CREATE TABLE IF NOT EXISTS `t_file` (\
		file_id VARCHAR(36) NOT NULL PRIMARY KEY, \
		file_name VARCHAR(255) NOT NULL, \
		file_path TEXT NOT NULL, \
		file_size BIGINT NOT NULL, \
		file_md5 CHAR(32) NOT NULL, \
		file_state VARCHAR(50) NOT NULL DEFAULT 'PENDING' \
	)ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
	if (mysql_query(_conn, sql_tfile.c_str()) != 0) {
		LOG_ERROR << "MySQL createTable t_file error: " << mysql_error(_conn);
		return false;
	}

	string sql_friendship = "CREATE TABLE IF NOT EXISTS `t_friendship` (\
		`userid` int NOT NULL, \
		`friend_id` int NOT NULL, \
		KEY `userid` (`userid`,`friend_id`) \
	)ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
	if (mysql_query(_conn, sql_friendship.c_str()) != 0) {
		LOG_ERROR << "MySQL createTable t_friendship error: " << mysql_error(_conn);
		return false;
	}

	string sql_offlinemsg = "CREATE TABLE IF NOT EXISTS `t_offlinemsg` (\
		`id` INT AUTO_INCREMENT PRIMARY KEY, \
		`sendTo` INT NOT NULL, \
		`sendFrom` INT NOT NULL, \
		`messageContent` TEXT NOT NULL, \
		`messageType` ENUM('text', 'friend_apply', 'vedio', 'audio', 'file') NOT NULL DEFAULT 'text', \
		`createTime` TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP \
	)ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
	if (mysql_query(_conn, sql_offlinemsg.c_str()) != 0) {
		LOG_ERROR << "MySQL createTable t_offlinemsg error: " << mysql_error(_conn);
		return false;
	}

	string sql_allgrp = "CREATE TABLE IF NOT EXISTS `t_allgrp` (\
		`id` int(11) NOT NULL AUTO_INCREMENT, \
		`groupname` varchar(50) CHARACTER SET latin1 NOT NULL, \
		`groupdesc` varchar(200) CHARACTER SET latin1 DEFAULT '', \
		PRIMARY KEY (`id`), \
		UNIQUE KEY `groupname` (`groupname`) \
	) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
	if (mysql_query(_conn, sql_allgrp.c_str()) != 0) {
		LOG_ERROR << "MySQL createTable t_allgrp error: " << mysql_error(_conn);
		return false;
	}

	string sql_grpuser = "CREATE TABLE IF NOT EXISTS `t_grpuser` (\
		`groupid` int(11) NOT NULL, \
		`userid` int(11) NOT NULL, \
		`grouprole` enum('creator','normal') CHARACTER SET latin1 DEFAULT NULL, \
		KEY `groupid` (`groupid`,`userid`) \
	) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
	if (mysql_query(_conn, sql_grpuser.c_str()) != 0) {
		LOG_ERROR << "MySQL createTable t_grpuser error: " << mysql_error(_conn);
		return false;
	}
	return true;
}


bool Connection::connect(string ip, unsigned short port, 
	string username, string password, string dbname)
{
	// 断开当前连接（如果已经连接）
    if (_conn != nullptr) {
        mysql_close(_conn);
        _conn = nullptr;
    }

    // 初始化新的连接
    _conn = mysql_init(nullptr);
    if (!_conn) {
        LOG_ERROR << "MySQL initialization failed";
        return false;
    }
	// 连接数据库
	MYSQL *p = mysql_real_connect(_conn, ip.c_str(), username.c_str(),
		password.c_str(), dbname.c_str(), port, nullptr, 0);
	
	if(!p){
		LOG_ERROR << "MySQL connection error: " << mysql_error(_conn);
	}
	return p != nullptr;
}

bool Connection::update(string sql)
{
	// 更新操作 insert、delete、update
	if (mysql_query(_conn, sql.c_str()))
	{
		LOG_ERROR << "sql update fail: " << sql << "error: " << mysql_error(_conn);
		return false;
	}
	return true;
}

MYSQL_RES* Connection::query(string sql)
{
	// 查询操作 select
	if (mysql_query(_conn, sql.c_str()))
	{
		LOG_ERROR << "query failed: " << sql << "error: " << mysql_error(_conn);;
		return nullptr;
	}
	return mysql_use_result(_conn);
}

// 获取连接
MYSQL *Connection::get_connection()
{
    return _conn;
}