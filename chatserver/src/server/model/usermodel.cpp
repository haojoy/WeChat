#include "usermodel.hpp"
#include <iostream>
using namespace std;

/*
+--------+-----------+----------+----------+-------------+---------+
| userid | avatar_id | username | password | tel         | state   |
+--------+-----------+----------+----------+-------------+---------+
|      4 | NULL      | haojoy   | 123      | 12345678910 | offline |
+--------+-----------+----------+----------+-------------+---------+
*/

// User表的增加方法
bool UserModel::insert(User &user)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into t_user(username, password, tel, state) values('%s', '%s', '%s', '%s')",
            user.getName().c_str(), user.getPwd().c_str(), user.getTel().c_str(), user.getState().c_str());

    shared_ptr<Connection> sp_conn = ConnectionPool::getConnectionPool()->getConnection();
  
    if (sp_conn != nullptr && sp_conn->update(sql))
    {
        // 获取插入成功的用户数据生成的主键id
        user.setId(mysql_insert_id(sp_conn->get_connection()));
        return true;
    }
    

    return false;
}

// 根据用户号码查询用户信息
User UserModel::query(int id)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select * from t_user where userid = %d", id);

    shared_ptr<Connection> sp_conn = ConnectionPool::getConnectionPool()->getConnection();
    if(sp_conn != nullptr){
        MYSQL_RES *res = sp_conn->query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[2]);
                user.setPwd(row[3]);
                user.setTel(row[4]);
                user.setState(row[5]);
                mysql_free_result(res);
                return user;
            }
        }
    }

    return User();
}

// 根据用户号码查询用户信息
User UserModel::query(string username)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select * from t_user where username = '%s';", username.c_str());

    shared_ptr<Connection> sp_conn = ConnectionPool::getConnectionPool()->getConnection();
    if(sp_conn != nullptr){
        MYSQL_RES *res = sp_conn->query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            
            if (row != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[2]);
                user.setPwd(row[3]);
                user.setTel(row[4]);
                user.setState(row[5]);
                mysql_free_result(res);
                return user;
            }
        }
    }

    return User();
}

vector<string> UserModel::queryuserinfo(string username)
{
    vector<string> vecquery;
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select `t_user`.userid, `t_file`.file_id, `t_file`.file_path, `t_file`.file_size, `t_file`.file_md5 from `t_user` inner join `t_file` on `t_user`.avatar_id = `t_file`.file_id where `t_user`.username = '%s';", username.c_str());

    shared_ptr<Connection> sp_conn = ConnectionPool::getConnectionPool()->getConnection();
    if(sp_conn != nullptr){
        MYSQL_RES *res = sp_conn->query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            
            if (row != nullptr)
            {
                vecquery.emplace_back(row[0]);
                vecquery.emplace_back(row[1]);
                vecquery.emplace_back(row[2]);
                vecquery.emplace_back(row[3]);
                vecquery.emplace_back(row[4]);
                mysql_free_result(res);
            }
        }
    }

    return vecquery;
}

// 更新用户的状态信息
bool UserModel::updateState(User user)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "update t_user set state = '%s' where userid = %d", user.getState().c_str(), user.getId());

    shared_ptr<Connection> sp_conn = ConnectionPool::getConnectionPool()->getConnection();
    if (sp_conn != nullptr && sp_conn->update(sql))
    {
        return true;
    }

    return false;
}

bool UserModel::updateAvatarId(int userid, string fileid)
{
    char sql[1024] = {0};
    sprintf(sql, "update t_user set avatar_id = '%s' where userid = %d", fileid.c_str(), userid);

    shared_ptr<Connection> sp_conn = ConnectionPool::getConnectionPool()->getConnection();
    if (sp_conn != nullptr && sp_conn->update(sql))
    {
        return true;
    }

    return false;
}

// 重置用户的状态信息
void UserModel::resetState()
{
    // 1.组装sql语句
    char sql[1024] = "update t_user set state = 'offline' where state = 'online'";

    shared_ptr<Connection> sp_conn = ConnectionPool::getConnectionPool()->getConnection();
    if(sp_conn != nullptr){
        sp_conn->update(sql);
    }
}