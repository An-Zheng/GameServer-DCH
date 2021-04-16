#pragma once
#include <string>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <malloc.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <fcntl.h>
#include <list>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <map>
#include "tool.h"
#include "./TcpListener.h"
#include "DB_Manager.h"
using namespace std;


class Channel;
class TcpListener;
class GameServer 
{
public:
	static GameServer& GetInstance() { return _instance; }

	int Init(int _maxLoad);
	void Run();
	bool Add_Channel(Channel& _Channel);
	void Del_Channel(Channel& _oChannel);
	void AddListener(TcpListener* _listener) { m_Listener = _listener; };
	TcpListener* GetListener() { return m_Listener; };
	static string Convert2Printable(std::string& _szData);
	int ConnectServer(string _ipAddress, int& _fd, int port);
	void UpdateGameServerInfo2LoginServer(int _deltaLoad);

	//-------------DB_OPERATION--------------
	void PlayerLogin(string _name, int _tcpFd);


	void PlayerLogout(string _name);

	PlayerInfo* FindPlayer(string name);
	map<string, PlayerInfo*>& GetOnlinePlayerList();

protected:

	GameServer();
	~GameServer();

private:
	static GameServer _instance;
	int m_iEpollFd = -1;
	list<Channel*> m_ChannelList;
	TcpListener* m_Listener;
	int m_curLoad;
	int m_maxLoad;
	DB_Manager m_db;
	int m_fd2LoginServer;
	map<string, PlayerInfo*> m_playerList;
};

