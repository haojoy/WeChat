#ifndef PUBLIC_H
#define PUBLIC_H

#include <iostream>
#include <string>
#include "json.hpp"
using json = nlohmann::json;
using namespace std;
/*
server和client的公共文件
*/

// 用户头像上传路径
#define _USER_AVATAR_UPLOAD_PATH ("/upload/userAvatar/")
#define _USER_FILE_UPLOAD_PATH ("/upload/userFile/")

enum RspStatus{
    REG_OK,
    REG_USER_EXIST,
    REG_ERR,

    LOGIN_OK,
    LOGIN_ONLINE,
    LOGIN_INVALID_USERORPWD,

    GET_FRIEND_INFO_SUCCESS,
    GET_FRIEND_INFO_NO_THIS_USER,

    ADD_FRIEND_ACCEPT,

    NOTNEEDUPLOAD,
    NEEDUPLOAD,
};

enum Message
{
    LOGIN_MSG = 1, // 登录消息
    LOGIN_MSG_ACK, // 登录响应消息
    LOGINOUT_MSG, // 注销消息
    REG_MSG, // 注册消息
    REG_MSG_ACK, // 注册响应消息
    ONE_CHAT_MSG, // 聊天消息
    ADD_FRIEND_REQ, // 添加好友消息
    ADD_FRIEND_RSP,

    CREATE_GROUP_MSG, // 创建群组
    ADD_GROUP_MSG, // 加入群组
    GROUP_CHAT_MSG, // 群聊天

    GET_FRIEND_INFO_REQ, // 获取待添加好友的信息
    GET_FRIEND_INFO_RSP, // 查找信息结果
    REFRESH_FRIEND_LIST,

    SET_AVATAR_RQ,
    SET_AVATAR_RS,
    SET_AVATAR_COMPLETE_NOTIFY,
    
};

struct RegisterMessage {
    string msgid;
    string name;
    string password;

    json to_json() const {
        return json{{"msgid", msgid}, {"name", name}, {"password", password}};
    }

    static RegisterMessage from_json(const json &js) {
        RegisterMessage msg;
        msg.msgid = js.at("msgid").get<string>();
        msg.name = js.at("name").get<string>();
        msg.password = js.at("password").get<string>();
        return msg;
    }
};

#endif