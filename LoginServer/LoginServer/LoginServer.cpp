#include "LoginServer.h"


LoginServer LoginServer::_instance;

LoginServer::LoginServer()
{
    m_iEpollFd = epoll_create(1);
}

LoginServer::~LoginServer()
{
	if (m_iEpollFd >= 0)
	{
		close(m_iEpollFd);
	}
}


void LoginServer::Init()
{
    epoll_event ev;
    ev.data.fd = STDIN_FILENO;
    ev.events = EPOLLIN;
    int ret = epoll_ctl(m_iEpollFd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);


}
void LoginServer::Run()
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
					LoginServer::GetInstance().Del_Channel(*_Channel);
					delete _Channel;
					break;
				}
				

			}
			if (0 != (EPOLLOUT & atmpEvent[i].events))
			{
				/*
				poChannel->FlushOut();
				if (false == poChannel->HasOutput())
				{
					Zinx_ClearChannelOut(*poChannel);
				}
				*/
			}

		}
	}
}

bool LoginServer::Add_Channel(Channel& _Channel)
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

void LoginServer::Del_Channel(Channel& _oChannel)
{
	//m_ChannelList.remove(&_oChannel);
	epoll_ctl(m_iEpollFd, EPOLL_CTL_DEL, _oChannel.GetFd(), NULL);
	_oChannel.Fini();
}

string LoginServer::Convert2Printable(std::string& _szData)
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



