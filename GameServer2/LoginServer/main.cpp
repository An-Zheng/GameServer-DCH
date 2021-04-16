#include <iostream>
#include "GameServer.h"
#include "tool.h"
using namespace std;


void SendGameServerInfo2LoginServer()
{
	auto backInfo = new pb::GameServerInfoBack;
}

int main()
{
	/*
    short s = 1;
	if (s == htons(s))
		cout << "´ó¶Ë" << endl;
	 else 
		cout << "Ð¡¶Ë" << endl;
	*/	
	GameServer& gameServer = GameServer::GetInstance();
	int maxLoad = 1000;
	gameServer.Init(maxLoad);
	gameServer.Run();
	

	return 0;
}