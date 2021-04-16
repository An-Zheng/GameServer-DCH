#pragma once
#include"tool.h"
class DB_Manager
{
	
public:
	~DB_Manager();
	void ConnectServer();
	void AddPlayer(PlayerInfo &_player);
	void DelPlayer(PlayerInfo& _player);
	int FindPlayer(string _playerName, PlayerInfo& _playerInfo);

private:
	MYSQL m_mysql;
};

