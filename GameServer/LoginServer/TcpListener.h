#pragma once
#include "GameServer.h"
#include "Channel.h"
#include "MessageHandler.h"
#include "tool.h"
#include "LoadBalancer.h"
using namespace std;


class TcpDataHandler;
class TcpListener : public Channel
{



public:
	TcpListener(unsigned short _usPort);
	virtual ~TcpListener();

	virtual bool handler();
	virtual bool Init();
	virtual void Fini();
	virtual int GetFd() ;
	//void UpdateServerList(int fd, string ip,int port,int curLoad,int maxLoad);
	//void DeleteServerFromList(int fd);
	ConsistentHash& GetConsistentHash() { return m_consistentHash; }
private:
	unsigned short m_usPort = 0;
	int m_fd = -1;
	map<int, GamerServerInfo> m_serverList;
	ConsistentHash m_consistentHash;
};

class TcpDataHandler : public Channel
{
private:
	int m_fd = -1;
public:
	TcpDataHandler(int _fd, string _clientIP, int _clientPort);
	virtual ~TcpDataHandler();

	virtual bool handler();
	virtual bool Init();
	virtual void Fini();
	virtual int GetFd();
	string m_playerName;
private:
	void parseMessage(string strRecv);
	
	string m_lastBuf;
	int m_clientPort;
	string m_clientIP;

};