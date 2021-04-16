#include "MessageHandler.h"
string gGServerIp;
int gPort;
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

			//游戏逻辑
		case GameMsgType::GAME_MSG_RESPONSE_IP:
		{
			auto pbmsg = dynamic_cast<pb::ResponseIP*>(single->mPbMsg);
			cout << pbmsg->ipaddress() << endl;
			gGServerIp = pbmsg->ipaddress();
			gPort = pbmsg->portnum();

			cout << "gGServerIp" << gGServerIp << endl;
			cout << "gPort" << gPort << endl;
			
		break;
		}
		
		case GameMsgType::GAMESERVER_2_CLIENT_LOGINBACK:
		{
			auto pbmsg = dynamic_cast<pb::LoginBack*>(single->mPbMsg);
			if (pbmsg->succ())
			{
				//cout player list

				for (int i = 0; i < pbmsg->playerlist().playernamelist_size(); i++)
				{
					cout << pbmsg->playerlist().playernamelist(i) << endl;
					cout << "login successfully" << endl;
				}
			}
			else
				cout <<" login fail" << endl;

		break;
		}
		case GameMsgType::GAMESERVER_2_CLIENT_PLAYERLISTBACK:
		{
			cout << "Player List:" << endl;
			auto pbmsg = dynamic_cast<pb::PlayerList*>(single->mPbMsg);
			for (int i = 0; i < pbmsg->playernamelist_size(); i++)
			{
				cout << pbmsg->playernamelist(i) << endl;
			}

			break;
		}
		case GameMsgType::GAMESERVER_2_CLIENT_ATTACKBACK:
		{

			auto pbmsg = dynamic_cast<pb::AttackResponse*>(single->mPbMsg);
			if (pbmsg->issucc()>0)
			{
				cout << "you win!" << endl;
			}
			else if (pbmsg->issucc() == -1)
			{
				cout << " didn't find player" << endl;
			}
			else 
			{
				cout << "you lose!" << endl;
			}
			break;
		}
		case GameMsgType::GAMESERVER_2_CLIENT_SENDMSGBACK:
		{
			auto pbmsg = dynamic_cast<pb::ChatBack*>(single->mPbMsg);
			if (pbmsg->succ() == 2)
			{
				cout << "didn't find player name" << endl;
			}
			break;
		}
		case GameMsgType::GAMESERVER_2_CLIENT_RECEIVEMSG:
		{
			auto pbmsg = dynamic_cast<pb::ReceiveChat*>(single->mPbMsg);
			cout << pbmsg->sendername() << ":  " << pbmsg->msg() << endl;
			break;
		}
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

	}
	free(pOut);
	delete _singleTLV;
	_singleTLV = nullptr;
	
}
