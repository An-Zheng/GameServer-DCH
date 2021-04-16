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
#include "./TcpListener.h"
using namespace std;


class Channel;
class TcpListener;
class LoginServer 
{
public:
	static LoginServer& GetInstance() { return _instance; }

	void Init();
	void Run();
	bool Add_Channel(Channel& _Channel);
	void Del_Channel(Channel& _oChannel);
	void AddListener(TcpListener* _listener) { m_Listener = _listener; };
	TcpListener* GetListener() { return m_Listener; };
	static string Convert2Printable(std::string& _szData);
protected:

	LoginServer();
	~LoginServer();

private:
	static LoginServer _instance;
	int m_iEpollFd = -1;
	list<Channel*> m_ChannelList;
	TcpListener* m_Listener;
};

