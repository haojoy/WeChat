#include "user.hpp"
#include "offlinemessagemodel.hpp"

// 存储用户的离线消息
void OfflineMsgModel::insert(int userid, string msg)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into t_offlinemsg values(%d, '%s')", userid, msg.c_str());

    shared_ptr<Connection> sp_conn = ConnectionPool::getConnectionPool()->getConnection();
    if(sp_conn != nullptr){
        sp_conn->update(sql);
    }

}

bool OfflineMsgModel::insert(OffLineMsg &offlinemsg)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into t_offlinemsg(sendTo, sendFrom, messageContent, messageType, createTime) values('%d', '%d', '%s', '%s', '%s')",
            offlinemsg.getSendTo(), offlinemsg.getSendFrom(), offlinemsg.getMsgContent().c_str(), offlinemsg.getMsgtype().c_str(), offlinemsg.getCreateTime().c_str());

    shared_ptr<Connection> sp_conn = ConnectionPool::getConnectionPool()->getConnection();

    if (sp_conn != nullptr && sp_conn->update(sql))
    {
        // 获取插入成功的用户数据生成的主键id
        offlinemsg.setId(mysql_insert_id(sp_conn->get_connection()));
        return true;
    }

    return false;
}

// 删除用户的离线消息
void OfflineMsgModel::remove(int userid)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "delete from offlinemessage where userid=%d", userid);

    shared_ptr<Connection> sp_conn = ConnectionPool::getConnectionPool()->getConnection();
    if(sp_conn != nullptr){
        sp_conn->update(sql);
    }
}

// 查询用户的离线消息
vector<string> OfflineMsgModel::query(int userid)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select message from offlinemessage where userid = %d", userid);

    vector<string> vec;
    shared_ptr<Connection> sp_conn = ConnectionPool::getConnectionPool()->getConnection();
    if(sp_conn != nullptr){
        MYSQL_RES *res = sp_conn->query(sql);
        if (res != nullptr)
        {
            // 把userid用户的所有离线消息放入vec中返回
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                vec.push_back(row[0]);
            }
            mysql_free_result(res);
            return vec;
        }
    }   
    
    return vec;
}