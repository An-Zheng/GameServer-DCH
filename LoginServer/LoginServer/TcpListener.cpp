#include "TcpListener.h"



TcpListener::TcpListener(unsigned short _usPort):m_usPort(_usPort),m_consistentHash(MAX_REPLICAS)
{
}

TcpListener::~TcpListener()
{
}

bool TcpListener::Init()
{
	bool bRet = false;
	int iListenFd = -1;
	struct sockaddr_in stServaddr;

	iListenFd = socket(AF_INET, SOCK_STREAM, 0);
	if (0 <= iListenFd)
	{
		memset(&stServaddr, 0, sizeof(stServaddr));
		stServaddr.sin_family = AF_INET;
		stServaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		stServaddr.sin_port = htons(m_usPort);

		int opt = 1;
		setsockopt(iListenFd, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(opt));

		if (0 == bind(iListenFd, (struct sockaddr*)&stServaddr, sizeof(stServaddr)))
		{
			if (0 == listen(iListenFd, 10))
			{
				bRet = true;
				m_fd = iListenFd;
			}
			else
			{
				perror("__FUNC__:listen:");
			}
		}
		else
		{
			perror("__FUNC__:bind:");
		}
	}
	else
	{
		perror("__FUNC__:socket:");
	}


	return bRet;
}

void TcpListener::Fini()
{
	if (0 <= m_fd)
	{
		close(m_fd);
		m_fd = -1;
	}

}

bool TcpListener::handler()
{

	
	bool bRet = false;

	int iDataFd = -1;
	struct sockaddr_in stClientAddr;
	socklen_t lAddrLen = sizeof(stClientAddr);
	
	iDataFd = accept(m_fd, (struct sockaddr*)&stClientAddr, &lAddrLen);
	if (0 <= iDataFd)
	{
		int clientPort = ntohs(stClientAddr.sin_port);
		string clinetIp = inet_ntoa(stClientAddr.sin_addr);
		auto _tcpDataHandler = new TcpDataHandler(iDataFd, clinetIp, clientPort);
		bRet = LoginServer::GetInstance().Add_Channel(*_tcpDataHandler);

	}
	else
	{
		perror("__FUNC__:accept:");
	}

	return bRet;
}



int TcpListener::GetFd()
{
	return m_fd;
}
void TcpListener::UpdateServerList(GamerServerInfo _serverInfo)
{
	
	m_serverList[_serverInfo.ip + to_string(_serverInfo.port)] = 0;

	LoginServer::GetInstance().GetListener()->GetConsistentHash().update(_serverInfo);
}
void TcpListener::DeleteServerFromList(string _ip, int _port)
{
	auto iter = m_serverList.find(_ip+ to_string(_port));
	if (iter != m_serverList.end())
	{
		m_serverList.erase(_ip + to_string(_port));
	}
	GamerServerInfo _serverInfo;
	_serverInfo.ip = _ip;
	_serverInfo.port = _port;
	LoginServer::GetInstance().GetListener()->GetConsistentHash().remove(_serverInfo);
}
/*
void TcpListener::UpdateServerList(int fd, string ip, int port, int curLoad, int maxLoad)
{

	m_serverList[fd].curLoad = curLoad;
	m_serverList[fd].maxLoad = maxLoad;
	m_serverList[fd].ip = ip;
	m_serverList[fd].port = port;
}

void TcpListener::DeleteServerFromList(int fd)
{
	m_serverList.erase(fd);
}

*/


TcpDataHandler::TcpDataHandler(int _fd, string _clientIP, int _clientPort):m_fd(_fd),m_clientIP(_clientIP),m_clientPort(_clientPort)
{
	/*
	string str = "192.168.0.9";
	send(m_fd, str.data(), str.size(), 0);
	*/
}

TcpDataHandler::~TcpDataHandler()
{

}

bool TcpDataHandler::handler()
{
	
	//cout << "hahaha" << endl;
	bool bRet = false;
	ssize_t iReadLen = -1;
	char acBuff[1024] = { 0 };
	string recvStr;
	while (0 < (iReadLen = recv(m_fd, acBuff, sizeof(acBuff), MSG_DONTWAIT)))
	{
		bRet = true;
		recvStr.append(acBuff, iReadLen);
	}
	/*
	std::cout << "<----------------------------------------->" << std::endl;
	std::cout << "recv from " << m_fd << ":" << LoginServer::Convert2Printable(recvStr) << std::endl;
	std::cout << "<----------------------------------------->" << std::endl;
	*/

	if (false == bRet)
	{
		LoginServer::GetInstance().GetListener()->DeleteServerFromList(m_clientIP,m_clientPort);
		SetChannelClose();
		//LoginServer::GetInstance().GetListener()->DeleteServerFromList(this->GetFd());
	}
	/*
	string _output = "hello world\n";
	char* pOut = (char*)calloc(1UL, _output.size());
	_output.copy(pOut, _output.size(), 0);
	send(m_fd, pOut, _output.size(), 0);
	*/
	parseMessage(recvStr);

	return false;
}

bool TcpDataHandler::Init()
{
	return true;
}

void TcpDataHandler::Fini()
{
	if (0 <= m_fd)
	{
		close(m_fd);
		m_fd = -1;
	}

}

int TcpDataHandler::GetFd()
{
	return m_fd;
}

void TcpDataHandler::parseMessage(string strRecv)
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
				msgHandler = new MessageHandler(this->GetFd(),m_clientIP,m_clientPort);
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
