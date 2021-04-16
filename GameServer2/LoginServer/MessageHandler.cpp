#include "MessageHandler.h"


string GameSingleTLV::serialize()
{

	string out;
	
	if (mPbMsg != nullptr)
	{
		out = mPbMsg->SerializeAsString();
	}
	
	return out;
}

GameSingleTLV::~GameSingleTLV()
{
	//cout << " GameSingleTLV::~GameSingleTLV" << endl;
	if (mPbMsg != nullptr)
	{
		//释放
		delete mPbMsg;
		mPbMsg = nullptr;
	}
}

GameSingleTLV::GameSingleTLV(GameMsgType type, string content)
	:m_messageType(type)
{


	//根据不同的消息id,然后反序列化为不同的protobuf对象

	switch (m_messageType)
	{
	case GAME_MSG_UPDATEGAMESERVER_INFO_BACK:
	{
		mPbMsg = new pb::GameServerInfoBack;
		break;
	}
	case GAME_MSG_UPDATEGAMESERVER_INFO:
	{
		mPbMsg = new pb::GameServerInfo;
		break;
	}
	case GAME_MSG_REQUEST_IP:
	{
		mPbMsg = new pb::RequestIP;
		break;
	}
	case GAME_MSG_RESPONSE_IP:
	{
		mPbMsg = new pb::ResponseIP;
		break;
	}
	case CLIENT_2_GAMESERVER_LOGIN:
	{
		mPbMsg = new pb::Login;
		break;
	}
	case GAMESERVER_2_CLIENT_LOGINBACK:
	{
		mPbMsg = new pb::LoginBack;
		break;
	}
	case CLIENT_2_GAMESERVER_PLAYERLIST:
	{
		mPbMsg = new pb::RequestList;
		break;
	}
	case GAMESERVER_2_CLIENT_PLAYERLISTBACK:
	{
		mPbMsg = new pb::PlayerList;
		break;
	}
	case CLIENT_2_GAMESERVER_ATTACK:
	{
		mPbMsg = new pb::AttackRequest;
		break;
	}
	case GAMESERVER_2_CLIENT_ATTACKBACK:
	{
		mPbMsg = new pb::AttackResponse;
		break;
	}
	case CLIENT_2_GAMESERVER_SENDMSG:
	{
		mPbMsg = new pb::Chat;
		break;
	}
	case GAMESERVER_2_CLIENT_SENDMSGBACK:
	{
		mPbMsg = new pb::ChatBack;
		break;
	}
	case GAMESERVER_2_CLIENT_RECEIVEMSG:
	{
		mPbMsg = new pb::ReceiveChat;
		break;
	}

	default:
		break;
	}
	//反序列化
	mPbMsg->ParseFromString(content);
	
}

MessageHandler::MessageHandler(int _tcpFd):m_tcpFD(_tcpFd)
{
}

MessageHandler::MessageHandler(TcpDataHandler* _tcpDataHndler, int _tcpFd, string _clientIP, int _clientPort): m_tcpDataHndler(_tcpDataHndler),m_tcpFD(_tcpFd),m_clientIp(_clientIP),m_clientPort(_clientPort)
{
}

MessageHandler::~MessageHandler()
{
	for (auto iter = m_MsgList.begin(); iter != m_MsgList.end(); ++iter)
	{
		delete *iter;
	}
}



void MessageHandler::AddSingleTLV(GameSingleTLV* _singleTLV)
{
	m_MsgList.push_back(_singleTLV);
}

void MessageHandler::HandleMessage()
{
	for (auto& single : m_MsgList)
	{
		switch (single->m_messageType)
		{

		case GAME_MSG_UPDATEGAMESERVER_INFO_BACK:
		{
			auto pbmsg = dynamic_cast<pb::GameServerInfoBack*>(single->mPbMsg);
			cout << "connect to login server ..." << endl;
			break;
		}

		case CLIENT_2_GAMESERVER_LOGIN:
		{
			auto pbmsg = dynamic_cast<pb::Login*>(single->mPbMsg);
			GameServer::GetInstance().PlayerLogin(pbmsg->playername(),m_tcpFD);

			//set player name for tcp datahandler
			m_tcpDataHndler->m_playerName = pbmsg->playername();

			map<string, PlayerInfo*> _playerList = GameServer::GetInstance().GetOnlinePlayerList();

			//fill in proto
			auto msg = new pb::LoginBack();
			auto msgPlayerList = new pb::PlayerList();
			msg->set_succ(1);
			int i = 0;
			for ( auto iter = _playerList.begin(); iter != _playerList.end() ;++iter )
			{
				pb::PlayerList* _playerNameList = msg->mutable_playerlist();
				string* playerName = _playerNameList->add_playernamelist() ;
				*playerName = iter->first;


				// fill in playerlist
				string* playerName2 = msgPlayerList->add_playernamelist();
				*playerName2 = iter->first;
			}
			//cout << msg->playerlist().playernamelist(0) << endl;
			auto singleTLV = new GameSingleTLV(GameMsgType::GAMESERVER_2_CLIENT_LOGINBACK, msg);
			SendMessage(singleTLV);
			
			//--- update player list in client ----------
			auto singleTLV2 = new GameSingleTLV(GameMsgType::GAMESERVER_2_CLIENT_PLAYERLISTBACK, msgPlayerList);
			for (auto iter = _playerList.begin(); iter != _playerList.end(); ++iter)
			{
				if (iter->first != pbmsg->playername())
				{
					SendMessage(singleTLV2,iter->second->m_tcpFd);
				}
				
			}
			delete singleTLV2;
			singleTLV2 = nullptr;
			//----------------update GameServer Info ------------

			GameServer::GetInstance().UpdateGameServerInfo2LoginServer( 1);
			break;
		}
		case CLIENT_2_GAMESERVER_PLAYERLIST:
		{
			auto msg = new pb::PlayerList();
			map<string, PlayerInfo*> _playerList = GameServer::GetInstance().GetOnlinePlayerList();

			for (auto iter = _playerList.begin(); iter != _playerList.end(); ++iter)
			{
				string* playerName = msg->add_playernamelist();
				*playerName = iter->first;
			}

			auto singleTLV = new GameSingleTLV(GameMsgType::GAMESERVER_2_CLIENT_PLAYERLISTBACK, msg);
			SendMessage(singleTLV);
			break;
		}
		case CLIENT_2_GAMESERVER_ATTACK:
		{
			auto pbmsg = dynamic_cast<pb::AttackRequest*>(single->mPbMsg);

			PlayerInfo* pAttacker = GameServer::GetInstance().FindPlayer(pbmsg->attackername());
			PlayerInfo* pOffender = GameServer::GetInstance().FindPlayer(pbmsg->offendername());

			auto msg = new pb::AttackResponse();
			if (pAttacker == nullptr || pOffender == nullptr)
			{	
				msg->set_issucc(-1);
			}
			else
			{
				int result = random() % 2;
				msg->set_issucc(result);
			}
			auto singleTLV = new GameSingleTLV(GameMsgType::GAMESERVER_2_CLIENT_ATTACKBACK, msg);
			SendMessage(singleTLV);
			break;
		}
		case CLIENT_2_GAMESERVER_SENDMSG:
		{

			auto pbmsg = dynamic_cast<pb::Chat*>(single->mPbMsg);

			PlayerInfo* pSender = GameServer::GetInstance().FindPlayer(pbmsg->sendername());
			PlayerInfo* pReceiver = GameServer::GetInstance().FindPlayer(pbmsg->receivername());

			auto msg = new pb::ChatBack();
			if (pSender == nullptr || pReceiver == nullptr)
			{
				msg->set_succ(2); //didn't find player name 
			}
			else
			{
				msg->set_succ(1);



				auto receiverMsg = new pb::ReceiveChat();
				receiverMsg->set_sendername(pbmsg->sendername());
				receiverMsg->set_msg(pbmsg->msg());
				auto singleTLV2 = new GameSingleTLV(GameMsgType::GAMESERVER_2_CLIENT_RECEIVEMSG, receiverMsg);
				SendMessage(singleTLV2, pReceiver->m_tcpFd);
				delete singleTLV2;
				singleTLV2 = NULL;

			}
			auto singleTLV = new GameSingleTLV(GameMsgType::GAMESERVER_2_CLIENT_SENDMSGBACK, msg);
			SendMessage(singleTLV);
			break;
		}
		default:
			break;
		}
	}
}

void MessageHandler::SendMessage(GameSingleTLV* _singleTLV)
{
	
	//protocol 协议层要在这里完成 role业务层 传过来的数据的协议封装
	string _output;

	//获取protobuf消息内容
	string content = _singleTLV->serialize();
	//消息长度
	//以小端的字节序输出
	int32_t type = _singleTLV->m_messageType;
	_output.push_back((char)(type & 0xff));
	_output.push_back((char)(type >> 8 & 0xff));
	_output.push_back((char)(type >> 16 & 0xff));
	_output.push_back((char)(type >> 24 & 0xff));


	int32_t len = content.size();
	_output.push_back((char)(len & 0xff));
	_output.push_back((char)(len >> 8 & 0xff));
	_output.push_back((char)(len >> 16 & 0xff));
	_output.push_back((char)(len >> 24 & 0xff));

	_output.append(content);



	char* pOut = (char*)calloc(1UL, _output.size());

	
	_output.copy(pOut, _output.size(), 0);
	if ((0 <= this->m_tcpFD) && (_output.size() == send(this->m_tcpFD, pOut, _output.size(), 0)))
	{
		//std::cout << "<----------------------------------------->" << std::endl;
		//std::cout << "send to " << this->m_tcpFD << ":" << GameServer::Convert2Printable(_output) << std::endl;
		//std::cout << "<----------------------------------------->" << std::endl;
	}
	free(pOut);
	delete _singleTLV;
	_singleTLV = nullptr;
	
}
void MessageHandler::SendMessage(GameSingleTLV* _singleTLV,int _tcpFd)
{

	//protocol 协议层要在这里完成 role业务层 传过来的数据的协议封装
	string _output;

	//获取protobuf消息内容
	string content = _singleTLV->serialize();
	//消息长度
	//以小端的字节序输出
	int32_t type = _singleTLV->m_messageType;
	_output.push_back((char)(type & 0xff));
	_output.push_back((char)(type >> 8 & 0xff));
	_output.push_back((char)(type >> 16 & 0xff));
	_output.push_back((char)(type >> 24 & 0xff));


	int32_t len = content.size();
	_output.push_back((char)(len & 0xff));
	_output.push_back((char)(len >> 8 & 0xff));
	_output.push_back((char)(len >> 16 & 0xff));
	_output.push_back((char)(len >> 24 & 0xff));

	_output.append(content);



	char* pOut = (char*)calloc(1UL, _output.size());


	_output.copy(pOut, _output.size(), 0);
	if ((0 <= _tcpFd) && (_output.size() == send(_tcpFd, pOut, _output.size(), 0)))
	{
		//std::cout << "<----------------------------------------->" << std::endl;
		//std::cout << "send to " << _tcpFd << ":" << GameServer::Convert2Printable(_output) << std::endl;
		//std::cout << "<----------------------------------------->" << std::endl;
	}
	free(pOut);


}