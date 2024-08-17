#include <fstream>
#include <sstream>
#include "connectionpool.h"
#include <muduo/base/Logging.h>

// 线程安全的懒汉单例函数接口
ConnectionPool* ConnectionPool::getConnectionPool()
{
	static ConnectionPool pool; // lock和unlock
	return &pool;
}

// 辅助函数：去除字符串的前后空白
string trim(const string& str)
{
    size_t start = str.find_first_not_of(" \t");
    size_t end = str.find_last_not_of(" \t");
    return (start == string::npos || end == string::npos) ? "" : str.substr(start, end - start + 1);
}

bool ConnectionPool::loadConfigFile()
{
	// 设置环境变量或绝对路径
	const char* configPath = getenv("MYSQL_CONF_PATH");
    if (!configPath)
    {
        LOG_ERROR << " mysql.conf MYSQL_CONF_PATH not set!";
        return false;
    }

    ifstream file(configPath);

    if (!file.is_open())
    {
        LOG_ERROR << "mysql.conf 文件不存在!";
        return false;
    }

    string line;
    while (getline(file, line))
    {
        // 忽略空行和注释行
        if (line.empty() || line.find('=') == string::npos)
            continue;
        if (line.back() == '\r') {
            line.pop_back();
        }
		
        istringstream iss(line);
        string key, value;
        if (getline(iss, key, '=') && getline(iss, value))
        {
            // 去除可能存在的前后空白
            key = trim(key);
            value = trim(value);

            if (key == "ip") _ip = value;
            else if (key == "port") _port = stoi(value);
            else if (key == "username") _username = value;
            else if (key == "password") _password = value;
            else if (key == "dbname") _dbname = value;
            else if (key == "initSize") _initSize = stoi(value);
            else if (key == "maxSize") _maxSize = stoi(value);
            else if (key == "maxIdleTime") _maxIdleTime = stoi(value);
            else if (key == "connectionTime") _connectionTimeout = stoi(value);
        }
    }

    return true;
}

// 连接池的构造
ConnectionPool::ConnectionPool()
{
	// 加载配置项了
	if (!loadConfigFile())
	{
		return;
	}

	// 创建初始数量的连接
	for (int i = 0; i < _initSize; ++i)
	{
		Connection *p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
		p->refreshAliveTime(); // 刷新一下开始空闲的起始时间
		_connectionQue.push(p);
		_connectionCnt++;
	}

	// 启动一个新的线程，作为连接的生产者 linux thread => pthread_create
	thread produce(std::bind(&ConnectionPool::produceConnectionTask, this));
	produce.detach();

	// 启动一个新的定时线程，扫描超过maxIdleTime时间的空闲连接，进行对于的连接回收
	thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));
	scanner.detach();
}

// 运行在独立的线程中，专门负责生产新连接
void ConnectionPool::produceConnectionTask()
{
	for (;;)
	{
		unique_lock<mutex> lock(_queueMutex);
		while (!_connectionQue.empty())
		{
			cv.wait(lock); // 队列不空，此处生产线程进入等待状态
		}

		// 连接数量没有到达上限，继续创建新的连接
		if (_connectionCnt < _maxSize)
		{
			Connection *p = new Connection();
			p->connect(_ip, _port, _username, _password, _dbname);
			p->refreshAliveTime(); // 刷新一下开始空闲的起始时间
			_connectionQue.push(p);
			_connectionCnt++;
		}

		// 通知消费者线程，可以消费连接了
		cv.notify_all();
	}
}

// 给外部提供接口，从连接池中获取一个可用的空闲连接
shared_ptr<Connection> ConnectionPool::getConnection()
{
	unique_lock<mutex> lock(_queueMutex);
	while (_connectionQue.empty())
	{
		// sleep
		if (cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(_connectionTimeout)))
		{
			if (_connectionQue.empty())
			{
				LOG_INFO << "获取空闲连接超时了...获取连接失败!";
					return nullptr;
			}
		}
	}

	/*
	shared_ptr智能指针析构时，会把connection资源直接delete掉，相当于
	调用connection的析构函数，connection就被close掉了。
	这里需要自定义shared_ptr的释放资源的方式，把connection直接归还到queue当中
	*/
	shared_ptr<Connection> sp(_connectionQue.front(), 
		[&](Connection *pcon) {
		// 这里是在服务器应用线程中调用的，所以一定要考虑队列的线程安全操作
		unique_lock<mutex> lock(_queueMutex);
		pcon->refreshAliveTime(); // 刷新一下开始空闲的起始时间
		_connectionQue.push(pcon);
	});

	_connectionQue.pop();
	cv.notify_all();  // 消费完连接以后，通知生产者线程检查一下，如果队列为空了，赶紧生产连接
	
	return sp;
}

// 扫描超过maxIdleTime时间的空闲连接，进行对于的连接回收
void ConnectionPool::scannerConnectionTask()
{
	for (;;)
	{
		// 通过sleep模拟定时效果
		this_thread::sleep_for(chrono::seconds(_maxIdleTime));

		// 扫描整个队列，释放多余的连接
		unique_lock<mutex> lock(_queueMutex);
		while (_connectionCnt > _initSize)
		{
			Connection *p = _connectionQue.front();
			if (p->getAliveeTime() >= (_maxIdleTime * 1000))
			{
				_connectionQue.pop();
				_connectionCnt--;
				delete p; // 调用~Connection()释放连接
			}
			else
			{
				break; // 队头的连接没有超过_maxIdleTime，其它连接肯定没有
			}
		}
	}
}