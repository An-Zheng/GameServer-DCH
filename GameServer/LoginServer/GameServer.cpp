#include "GameServer.h"


GameServer GameServer::_instance;

GameServer::GameServer()
{
    m_iEpollFd = epoll_create(1);
}

GameServer::~GameServer()
{
	if (m_iEpollFd >= 0)
	{
		close(m_iEpollFd);
	}

	for (auto iter = m_playerList.begin(); iter != m_playerList.end(); ++iter)
	{
		delete iter->second;
		iter->second = NULL;
	}
	
}

void GameServer::UpdateGameServerInfo2LoginServer(int _deltaLoad)
{
	//send GameServer Infor to Login Server
	m_curLoad = m_curLoad + _deltaLoad;
	auto backInfo = new pb::GameServerInfo;
	backInfo->set_ipaddress(GAMESERVER_IP);
	backInfo->set_portnum(GAMESERVER_PORT);
	backInfo->set_curload(m_curLoad);
	backInfo->set_maxload(m_maxLoad);

	auto singleTLV = new GameSingleTLV(GameMsgType::GAME_MSG_UPDATEGAMESERVER_INFO, backInfo);
	MessageHandler loginMsgHandler = MessageHandler(m_fd2LoginServer);
	loginMsgHandler.SendMessage(singleTLV);


}

void GameServer::PlayerLogin(string _name, int _tcpFd)
{

	if (FindPlayer(_name) == NULL)
	{
		PlayerInfo* _playerInfo = new PlayerInfo(_name, _tcpFd);
		m_playerList[_name] = _playerInfo;
		if (m_db.FindPlayer(_name, *_playerInfo) != 0)
		{
			m_db.AddPlayer(*_playerInfo);
		}
	}



}

void GameServer::PlayerLogout(string _name)
{
	if (m_playerList.find(_name) != m_playerList.end())
	{
		m_playerList.erase(_name);
	}
}

PlayerInfo* GameServer::FindPlayer(string name)
{
	auto iter = m_playerList.find(name);
	if (iter != m_playerList.end())
	{
		return iter->second;
	}
	return nullptr;
}

map<string, PlayerInfo*>& GameServer::GetOnlinePlayerList()
{
	return m_playerList;
}




int GameServer::Init(int _maxLoad)
{   //init listener
	auto listen = new TcpListener(GAMESERVER_PORT);
	Add_Channel(*listen);
	AddListener(listen);

	//----------------------------connect Login Server---------------
	m_curLoad = 0;
	m_maxLoad = _maxLoad;
	m_fd2LoginServer = 0;
	if (ConnectServer(LOGINSERVER_IP, m_fd2LoginServer, LOGINSERVER_PORT) != 0)
	{
		cout << "Connect Login Server fail" << endl;
		return 0;
	}

	auto _tcpDataHandler = new TcpDataHandler(m_fd2LoginServer, LOGINSERVER_IP, LOGINSERVER_PORT);
	int bRet = Add_Channel(*_tcpDataHandler);

	UpdateGameServerInfo2LoginServer(0);
	//-------------connect DB -------------

	m_db.ConnectServer();

}
void GameServer::Run()
{
	int iEpollRet = -1;

	while (1)
	{
		struct epoll_event atmpEvent[100];
		iEpollRet = epoll_wait(m_iEpollFd, atmpEvent, 100, -1);
		if (-1 == iEpollRet)
		{
			if (EINTR == errno)
			{
				continue;
			}
			else
			{
				break;
			}
		}
		for (int i = 0; i < iEpollRet; i++)
		{
			Channel* _Channel = static_cast<Channel*>(atmpEvent[i].data.ptr);
			if (0 != (EPOLLIN & atmpEvent[i].events))
			{
				_Channel->handler();
				
				if (true == _Channel->ChannelNeedClose())
				{
					GameServer::GetInstance().Del_Channel(*_Channel);
					delete _Channel;
					break;
				}
				

			}

		}
	}
}

bool GameServer::Add_Channel(Channel& _Channel)
{
	bool bRet = false;

	if (true == _Channel.Init())
	{
		struct epoll_event stEvent;
		stEvent.events = EPOLLIN;
		stEvent.data.ptr = &_Channel;

		if (0 == epoll_ctl(m_iEpollFd, EPOLL_CTL_ADD, _Channel.GetFd(), &stEvent))
		{
			m_ChannelList.push_back(&_Channel);
			bRet = true;
		}
	}

	return bRet;
}

void GameServer::Del_Channel(Channel& _oChannel)
{
	//m_ChannelList.remove(&_oChannel);
	epoll_ctl(m_iEpollFd, EPOLL_CTL_DEL, _oChannel.GetFd(), NULL);
	_oChannel.Fini();
}

string GameServer::Convert2Printable(std::string& _szData)
{
	char* pcTemp = (char*)calloc(1UL, _szData.size() * 3 + 1);
	int iCurPos = 0;

	if (NULL != pcTemp)
	{
		for (int i = 0; i < _szData.size(); ++i)
		{
			iCurPos += sprintf(pcTemp + iCurPos, "%02X ", (unsigned char)_szData[i]);
		}
		pcTemp[iCurPos] = 0;
	}

	std::string szRet = std::string(pcTemp);
	free(pcTemp);

	return szRet;
}

int GameServer::ConnectServer(string _ipAddress, int& _fd, int port)
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



