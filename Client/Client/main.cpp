#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#include <error.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include "MessageHandler.h"

using namespace std;

string m_lastBuf;
string myname;
extern string gGServerIp;
extern int gPort;
void parseMessage(string strRecv,int fd)
{
	//通道层TCP读取的数据会传输到这里来进行协议解析
//每次都将新读到的数据追加到上一次的数据
	m_lastBuf.append(strRecv);
	MessageHandler* msgHandler = nullptr;
	//数据长度至少要大于8才做处理
	while (m_lastBuf.size() >= 8)
	{
		// 0x01 0x02 0x03 0x04    ->   0x04030201
		// 

		int msgType =
			m_lastBuf[0] |
			m_lastBuf[1] << 8 |
			m_lastBuf[2] << 16 |
			m_lastBuf[3] << 24;
		/*
		int len =
			m_lastBuf[3] |
			m_lastBuf[2] << 8 |
			m_lastBuf[1] << 16 |
			m_lastBuf[0] << 24;
		*/
		int len =
			m_lastBuf[4] |
			m_lastBuf[5] << 8 |
			m_lastBuf[6] << 16 |
			m_lastBuf[7] << 24;
		cout << "len:" << len << " msgType:" << msgType << endl;
		//判断消息内容长度够不够  01000000010000000102000000020000000202
		if (m_lastBuf.size() - 8 >= len)
		{
			//从字符串取子串就可以   该子串是protobuf的一个消息
			string msgContent = m_lastBuf.substr(8, len);
			//buf要清理掉这个报文
			m_lastBuf.erase(0, 8 + len);

			//产生一个SingletTLV

			auto msg = new GameSingleTLV((GameMsgType)msgType, msgContent);

			if (msgHandler == nullptr)
			{
				msgHandler = new MessageHandler(fd);
			}
			//将singletlv加到GameMsg
			msgHandler->AddSingleTLV(msg);

		}
		else
		{
			//剩下长度不够,就不要继续循环等待下一次数据到来再出来
			break;
		}
	}

	//handle message
	if (msgHandler != nullptr)
	{
		msgHandler->HandleMessage();
		delete msgHandler;
		msgHandler = nullptr;
	}
}
void ReceiveMessage(int fd)
{

    ssize_t iReadLen = -1;
    char acBuff[2048] = { 0 };
    string recvStr;
	iReadLen = recv(fd, acBuff, sizeof(acBuff), 0);
    if(iReadLen == -1)
	{
		cout << "ReceiveMessage error" << endl;
		return;
    }
	recvStr.append(acBuff, iReadLen);

    parseMessage(recvStr,fd);

}
int ConnectServer(string _ipAddress, int& _fd,int port) 
{
	//第一步：初始化一个socket实例
    _fd = socket(AF_INET, SOCK_STREAM, 0);
	//第二步：定义一个IP地址结构并设置值
	struct sockaddr_in addr;
	//内存初始化，将addr变量指向的内存签n个字节用0进行初始化填充
	memset(&addr, 0, sizeof(addr));
	//设置采用的协议为TCP/IP协议
	addr.sin_family = AF_INET;
	//设置端口号
	addr.sin_port = htons(port);
	//设置IP地址
	addr.sin_addr.s_addr = inet_addr(_ipAddress.data());

	//第三步：开始连接服务端
	if (connect(_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		printf("connect fail %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	return 0;
}
void Login(int _fd)
{
	string _name;
	cin >> _name;
	myname = _name;
	auto msg = new pb::Login();
	msg->set_playername(_name.data());
	auto singleTLV = new  GameSingleTLV(GameMsgType::CLIENT_2_GAMESERVER_LOGIN, msg);
	MessageHandler loginMsgHandler = MessageHandler(_fd);
	loginMsgHandler.SendMessage(singleTLV);
	ReceiveMessage(_fd);
}

void ShowPlayerList(int _fd)
{
	auto msg = new pb::RequestList();
	msg->set_id(1);
	auto singleTLV = new GameSingleTLV(GameMsgType::CLIENT_2_GAMESERVER_PLAYERLIST, msg);
	MessageHandler loginMsgHandler = MessageHandler(_fd);
	loginMsgHandler.SendMessage(singleTLV);
	//ReceiveMessage(_fd);
}
void Attack(int _fd, string _offender)
{
	auto msg = new pb::AttackRequest();
	msg->set_attackername(myname.data());
	msg->set_offendername(_offender.data());
	auto singleTLV = new GameSingleTLV(GameMsgType::CLIENT_2_GAMESERVER_ATTACK, msg);
	MessageHandler loginMsgHandler = MessageHandler(_fd);
	loginMsgHandler.SendMessage(singleTLV);
	//ReceiveMessage(_fd);
}

void SendChatMsg(int _fd, string _receiver,string _content)
{
	auto msg = new pb::Chat();
	msg->set_sendername(myname.data());
	msg->set_receivername(_receiver.data());
	msg->set_msg(_content);
	auto singleTLV = new GameSingleTLV(GameMsgType::CLIENT_2_GAMESERVER_SENDMSG, msg);
	MessageHandler loginMsgHandler = MessageHandler(_fd);
	loginMsgHandler.SendMessage(singleTLV);
	//ReceiveMessage(_fd);
}


int main()
{
	//Connect to Login Server
	int loginFd = 0;
	ConnectServer("192.168.18.128",loginFd,8899);
	auto msg =  new pb::RequestIP();
	msg->set_id(1);
	auto singleTLV = new GameSingleTLV(GameMsgType::GAME_MSG_REQUEST_IP, msg);
	MessageHandler loginMsgHandler = MessageHandler(loginFd);
	loginMsgHandler.SendMessage(singleTLV);


	ReceiveMessage(loginFd);

	// Connect to GameServer

	int gameServerFd = 0;
	ConnectServer(gGServerIp, gameServerFd,gPort);

	// Login
	Login(gameServerFd);

	string _input;
	bool isClientWork = true;
	//创建子进程
	int pid = fork();
	if (pid < 0)
	{
		perror("fork error");
		exit(-1);
	}
	else if (pid == 0)
	{

		while (isClientWork)
		{

			cout << "------Player List----------" << endl;
			ShowPlayerList(gameServerFd);
			cout << "please input commands: '1' to attack " << endl;
			cout << "please input commands: '2' to refresh player list " << endl;
			cout << "please input commands: '3' to chat with a player " << endl;
			cout << "input 'exit' to exit " << endl;
			cin >> _input;
			if (_input == "1")
			{
				cout << "please input a player name you wanna attack." << endl;
				cin >> _input;
				Attack(gameServerFd,_input);
			}
			else if(_input == "exit")
			{

				isClientWork = false;
			}
			else if (_input == "3" )
			{
				cout << "please input a player name you wanna chat." << endl;
				string recervername;
				cin >> recervername;
				
				while (isClientWork)
				{
					cout << "please input msg ... " << endl;
					cin >> _input;

					if (_input == "exit")
						break;
					else
						SendChatMsg(gameServerFd,recervername,_input);
				}


			}
			else
			{
				continue;
			}

		}
	

	}
	else 
	{
		while (isClientWork)
		{
			ReceiveMessage(gameServerFd);
		}
	}

    
    
   
   

    
    close(loginFd);
	close(gameServerFd);
    getchar(); 
    return EXIT_SUCCESS;
}