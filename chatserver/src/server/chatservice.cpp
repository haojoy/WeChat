#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <vector>
using namespace std;
using namespace muduo;

// 获取单例对象的接口函数
ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

// 注册消息以及对应的Handler回调操作
ChatService::ChatService()
{
/*
#define XX(str, func) {\
    auto call = std::bind(&ChatService::func, this, _1, _2, _3); \
        _msgHandlerMap.insert({ str, call });}
XX(LOGIN_MSG, login);
XX(LOGINOUT_MSG, loginout);
XX(REG_MSG, userRegister);
XX(ONE_CHAT_MSG, oneChat);
XX(ADD_FRIEND_MSG, addFriendReq);
XX(CREATE_GROUP_MSG, createGroup);
XX(ADD_GROUP_MSG, addGroup);
XX(GROUP_CHAT_MSG, groupChat);
#undef XX
*/
    // 用户基本业务管理相关事件处理回调注册
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::userRegister, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_REQ, std::bind(&ChatService::addFriendReq, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_RSP, std::bind(&ChatService::addFriendRsp, this, _1, _2, _3)});

    // 群组业务管理相关事件处理回调注册
    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});

    _msgHandlerMap.insert({GET_FRIEND_INFO_REQ, std::bind(&ChatService::getFriendInfoReq, this, _1, _2, _3)});


    // 连接redis服务器
    if (_redis.connect())
    {
        // 设置上报消息的回调
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
    }
}

// 服务器异常，业务重置方法
void ChatService::reset()
{
    // 把online状态的用户，设置成offline
    _userModel.resetState();
}

// 获取消息对应的处理器
MsgHandler ChatService::getHandler(int msgid)
{
    // 记录错误日志，msgid没有对应的事件处理回调
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {
        // 返回一个默认的处理器，空操作
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp) {
            LOG_ERROR << "msgid:" << msgid << " can not find handler!";
        };
    }
    else
    {
        return _msgHandlerMap[msgid];
    }
}

// 处理登录业务  id  pwd   pwd
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    string username = js["name"];
    string pwd = js["password"];

    
    User user = _userModel.query(username);
    if (user.getName() == username && user.getPwd() == pwd)
    {
        int id = user.getId();
        if (user.getState() == "online")
        {
            // 该用户已经登录，不允许重复登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = LOGIN_ONLINE;
            conn->send(response.dump());
        }
        else
        {
            // 登录成功，记录用户连接信息
            {
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({id, conn});
            }

            // id用户登录成功后，向redis订阅channel(id)
            _redis.subscribe(id); 

            // 登录成功，更新用户状态信息 state offline=>online
            user.setState("online");
            _userModel.updateState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = LOGIN_OK;
            response["userid"] = user.getId();
            response["name"] = user.getName();
            // 查询该用户是否有离线消息
            vector<string> vec = _offlineMsgModel.query(id);
            if (!vec.empty())
            {
                response["offlinemsg"] = vec;
                // 读取该用户的离线消息后，把该用户的所有离线消息删除掉
                _offlineMsgModel.remove(id);
            }

            // 查询该用户的好友信息并返回
            vector<User> userVec = _friendModel.query(id);
            if (!userVec.empty())
            {
                vector<string> vec2;
                for (User &user : userVec)
                {
                    json js;
                    js["userid"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }

            // 查询用户的群组信息
            vector<Group> groupuserVec = _groupModel.queryGroups(id);
            if (!groupuserVec.empty())
            {
                // group:[{groupid:[xxx, xxx, xxx, xxx]}]
                vector<string> groupV;
                for (Group &group : groupuserVec)
                {
                    json grpjson;
                    grpjson["grpid"] = group.getId();
                    grpjson["groupname"] = group.getName();
                    grpjson["groupdesc"] = group.getDesc();
                    vector<string> userV;
                    for (GroupUser &user : group.getUsers())
                    {
                        json js;
                        js["userid"] = user.getId();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        js["role"] = user.getRole();
                        userV.push_back(js.dump());
                    }
                    grpjson["users"] = userV;
                    groupV.push_back(grpjson.dump());
                }

                response["groups"] = groupV;
            }

            conn->send(response.dump());
        }
    }
    else
    {
        // 该用户不存在，用户存在但是密码错误，登录失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = LOGIN_INVALID_USERORPWD;
        conn->send(response.dump());
    }
}

// 处理注册业务  name  password
void ChatService::userRegister(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    string name = js["name"];
    string tel = js["tel"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setTel(tel);
    user.setPwd(pwd);

    // 判断该用户是否存在
    User usertmp = _userModel.query(name);
    if (usertmp.getId() != -1){
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = REG_USER_EXIST;
        conn->send(response.dump());
        return;
    }
    bool state = _userModel.insert(user);
    if (state)
    {
        // 注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = REG_OK;
        conn->send(response.dump());
    }
    else
    {
        // 注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = REG_ERR;
        conn->send(response.dump());
    }
}

// 处理注销业务
void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();

    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if (it != _userConnMap.end())
        {
            _userConnMap.erase(it);
        }
    }

    // 用户注销，相当于就是下线，在redis中取消订阅通道
    _redis.unsubscribe(userid); 

    // 更新用户的状态信息
    User user(userid, "", "", "offline");
    _userModel.updateState(user);
}

// 处理客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        lock_guard<mutex> lock(_connMutex);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it)
        {
            if (it->second == conn)
            {
                // 从map表删除用户的链接信息
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }

    // 用户注销，相当于就是下线，在redis中取消订阅通道
    _redis.unsubscribe(user.getId()); 

    // 更新用户的状态信息
    if (user.getId() != -1)
    {
        user.setState("offline");
        _userModel.updateState(user);
    }
}

// 一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toid = js["receiverid"].get<int>();

    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end())
        {
            // toid在线，转发消息   服务器主动推送消息给toid用户
            it->second->send(js.dump());
            return;
        }
    }

    // 查询toid是否在线 
    User user = _userModel.query(toid);
    if (user.getState() == "online")
    {
        _redis.publish(toid, js.dump());
        return;
    }

    int senderid =  js["senderid"].get<int>();
    OffLineMsg offlinemsg;
    offlinemsg.setSendTo(toid);
    offlinemsg.setSendFrom(senderid);
    offlinemsg.setMsgContent(js["msginfo"]);
    offlinemsg.setMsgType("text");
    offlinemsg.setCreateTime();
    _offlineMsgModel.insert(offlinemsg);

}

// 获取待添加好友信息
void ChatService::getFriendInfoReq(const TcpConnectionPtr &conn, json &js, Timestamp time){
    string name = js["name"];
    // 判断该用户是否存在
    User user = _userModel.query(name);
    json response;
    if (user.getId() != -1){
        response["errno"] = GET_FRIEND_INFO_SUCCESS;
        response["msgid"] = GET_FRIEND_INFO_RSP;
        response["userid"] = user.getId();
        response["name"] = user.getName();
        conn->send(response.dump());
        return;
    }
    response["errno"] = GET_FRIEND_INFO_NO_THIS_USER;
    response["msgid"] = GET_FRIEND_INFO_RSP;
    conn->send(response.dump());
}

// 添加好友业务 msgid id friendid
void ChatService::addFriendReq(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["senderid"].get<int>();
    string sendername = js["sendername"];
    int friendid = js["receiverid"].get<int>();
    string receivername = js["receivername"];

    if (_userConnMap.find(friendid) != _userConnMap.end()) { // 对方在线
        json response;
        response["msgid"] = ADD_FRIEND_REQ;
        response["senderid"] = userid;
        response["sendername"] = sendername;
        response["receiverid"] = friendid;
        response["receivername"] = receivername;
        _userConnMap[friendid]->send(response.dump());
    }else{
        OffLineMsg offlinemsg;
        offlinemsg.setSendTo(friendid);
        offlinemsg.setSendFrom(userid);
        offlinemsg.setMsgContent(sendername);
        offlinemsg.setMsgType("friend_apply");
        offlinemsg.setCreateTime();
        _offlineMsgModel.insert(offlinemsg);
    }
}

void ChatService::addFriendRsp(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int status = js["result"].get<int>();
    int userid = js["senderid"].get<int>();
    int friendid = js["receiverid"].get<int>();
    if(status == ADD_FRIEND_ACCEPT){
        _friendModel.insert(userid, friendid);
        _friendModel.insert(friendid, userid);
    }
    User user = _userModel.query(userid);
    User friendinfo = _userModel.query(friendid);

    if (_userConnMap.find(friendid) != _userConnMap.end() && user.getId() != -1) { // 对方在线
        json response;
        response["msgid"] = REFRESH_FRIEND_LIST;
        response["userid"] = user.getId();
        response["username"] = user.getName();
        response["state"] = user.getState();
        _userConnMap[friendid]->send(response.dump());
    }
    if(friendinfo.getId() != -1){
        json response;
        response["msgid"] = REFRESH_FRIEND_LIST;
        response["userid"] = friendinfo.getId();
        response["username"] = friendinfo.getName();
        response["state"] = friendinfo.getState();
        conn->send(response.dump());
    }

}

// 创建群组业务
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    // 存储新创建的群组信息
    Group group(-1, name, desc);
    if (_groupModel.createGroup(group))
    {
        // 存储群组创建人信息
        _groupModel.addGroup(userid, group.getId(), "creator");
    }
}

// 加入群组业务
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "normal");
}

// 群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);

    lock_guard<mutex> lock(_connMutex);
    for (int id : useridVec)
    {
        auto it = _userConnMap.find(id);
        if (it != _userConnMap.end())
        {
            // 转发群消息
            it->second->send(js.dump());
        }
        else
        {
            // 查询toid是否在线 
            User user = _userModel.query(id);
            if (user.getState() == "online")
            {
                _redis.publish(id, js.dump());
            }
            else
            {
                // 存储离线群消息
                _offlineMsgModel.insert(id, js.dump());
            }
        }
    }
}

// 从redis消息队列中获取订阅的消息
void ChatService::handleRedisSubscribeMessage(int userid, string msg)
{
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if (it != _userConnMap.end())
    {
        it->second->send(msg);
        return;
    }

    // 存储该用户的离线消息
    _offlineMsgModel.insert(userid, msg);
}