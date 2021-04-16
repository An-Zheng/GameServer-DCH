#include "DB_Manager.h"

DB_Manager::~DB_Manager()
{

    mysql_close(&m_mysql);
}

void DB_Manager::ConnectServer()
{
    mysql_init(&m_mysql);

    if (mysql_real_connect(&m_mysql, "localhost", "root", "root", "mydb1", 3306, NULL, 0) == NULL)
    {
        cout<<" connect DB fail....."<<endl;
    }
    else
        cout << " connect DB ....." << endl;
}

void DB_Manager::AddPlayer(PlayerInfo& _player)
{
    //���� 
    string sql = "insert into players value('"+ _player.m_playerName+"')";
    //ִ��sql���
    mysql_query(&m_mysql, sql.c_str());
}

void DB_Manager::DelPlayer(PlayerInfo& _player)
{
}

int DB_Manager::FindPlayer(string _playerName, PlayerInfo &_playerInfo)
{
    
    MYSQL_RES* res;
    MYSQL_ROW row;
    string sql = "select playername from players where playername = '" + _playerName + "'";
    //ִ��sql���
    mysql_query(&m_mysql, sql.c_str());

    res = mysql_store_result(&m_mysql);
    while (row = mysql_fetch_row(res))
    {
        _playerInfo.m_playerName = row[0];
    }
    if (_playerInfo.m_playerName == "")
    {
        return -1;
    }
	return 0;
}
