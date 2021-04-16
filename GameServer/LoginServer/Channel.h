#pragma once
#include <string>
using namespace std;
class Channel
{
public:
	virtual bool handler()=0;
	virtual bool Init() = 0;
	virtual int GetFd()= 0;
	virtual void Fini() = 0;
	void SetChannelClose() { m_NeedClose = true; }
	bool ChannelNeedClose() { return m_NeedClose; }
private:
	bool m_NeedClose = false;
};

