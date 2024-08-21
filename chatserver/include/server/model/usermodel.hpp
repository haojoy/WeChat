#ifndef USERMODEL_H
#define USERMODEL_H

#include "user.hpp"
#include <vector>
// User表的数据操作类
class UserModel {
public:
    // User表的增加方法
    bool insert(User &user);

    // 根据用户号码查询用户信息
    User query(int id);

    User query(string username);
    vector<string> queryuserinfo(string username);
    // 更新用户的状态信息
    bool updateState(User user);

    bool updateAvatarId(int userid, string fileid);

    // 重置用户的状态信息
    void resetState();
};

#endif