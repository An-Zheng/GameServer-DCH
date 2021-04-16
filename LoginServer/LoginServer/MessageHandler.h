#pragma once
#include "LoginServer.h"
#include "allmsg.pb.h"


//�����߼���Ϣ������
enum GameMsgType {
	CLIENT_2_GAMESERVER_LOGIN = 1,
	GAMESERVER_2_CLIENT_LOGINBACK = 2,

	CLIENT_2_GAMESERVER_PLAYERLIST = 3,
	GAMESERVER_2_CLIENT_PLAYERLISTBACK = 4,


	CLIENT_2_GAMESERVER_ATTACK = 9,
	GAMESERVER_2_CLIENT_ATTACKBACK = 10,

	CLIENT_2_GAMESERVER_SENDMSG = 11,
	GAMESERVER_2_CLIENT_SENDMSGBACK = 12,
	GAMESERVER_2_CLIENT_RECEIVEMSG = 13,

	GAME_MSG_REQUEST_IP = 7,
	GAME_MSG_RESPONSE_IP = 8,
	GAME_MSG_UPDATEGAMESERVER_INFO_BACK = 5,
	GAME_MSG_UPDATEGAMESERVER_INFO = 6,

};
//������������洢һ���߼�����,���ճ��֮�������ĵ���һ��ҵ����
class GameSingleTLV
{
public:
	int gInt;
	GameMsgType m_messageType;

	//����һ����������ָ��,�����洢��ͬ���͵�����
	::google::protobuf::Message* mPbMsg;

	string serialize();

	~GameSingleTLV();
	GameSingleTLV(GameMsgType type, std::string content);
	GameSingleTLV(GameMsgType type, ::google::protobuf::Message* pbmsg) :m_messageType(type), mPbMsg(pbmsg) {}

};
class MessageHandler
{
public:
	MessageHandler(int _tcpFd, string _clientIP,int _clientPort);
	~MessageHandler();
	//parse the received string, seperate it and gain sub contents and input these contents into message list.  

	void AddSingleTLV(GameSingleTLV* _singleTLV);
	void HandleMessage();
	
	//send message to server
	void SendMessage(GameSingleTLV* _singleTLV);
private:
	int m_tcpFD;
	string m_clientIp;
	int m_clientPort;
	std::list<GameSingleTLV*> m_MsgList;
};

