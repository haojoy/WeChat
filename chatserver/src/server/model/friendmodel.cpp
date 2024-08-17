#include "friendmodel.hpp"

// 添加好友关系
void FriendModel::insert(int userid, int friendid)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into t_friendship values(%d, %d)", userid, friendid);

    shared_ptr<Connection> sp_conn = ConnectionPool::getConnectionPool()->getConnection();
    if(sp_conn != nullptr){
        sp_conn->update(sql);
    }
	
}

// 返回用户好友列表
vector<User> FriendModel::query(int userid)
{
    // 1.组装sql语句
    char sql[1024] = {0};

    sprintf(sql, "select a.userid,a.username,a.state from t_user a inner join t_friendship b on b.friend_id = a.userid where b.userid=%d", userid);

    vector<User> vec;
    shared_ptr<Connection> sp_conn = ConnectionPool::getConnectionPool()->getConnection();
    if(sp_conn != nullptr){
        MYSQL_RES *res = sp_conn->query(sql);
        if (res != nullptr)
        {
            // 把userid用户的所有离线消息放入vec中返回
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[2]);
                user.setPwd(row[3]);
                user.setTel(row[4]);
                user.setState(row[5]);
                vec.push_back(user);
            }
            mysql_free_result(res);
            return vec;
        }
    }
    
    
    return vec;
}