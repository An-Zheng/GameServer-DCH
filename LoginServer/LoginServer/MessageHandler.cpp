#include "MessageHandler.h"
#include "tool.h"

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

MessageHandler::MessageHandler(int _tcpFd, string _clientIP, int _clientPort):m_tcpFD(_tcpFd),m_clientIp(_clientIP),m_clientPort(_clientPort)
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

		//game client requests game sever IP
		case GameMsgType::GAME_MSG_REQUEST_IP:
		{

			string tmpClientInfo = m_clientIp + to_string(m_clientPort);
			GamerServerInfo serverInfo = LoginServer::GetInstance().GetListener()->GetConsistentHash().get(tmpClientInfo);
			auto msg = new pb::ResponseIP;
			msg->set_ipaddress(serverInfo.ip);
			msg->set_portnum(serverInfo.port);
			auto singleTLV = new GameSingleTLV(GameMsgType::GAME_MSG_RESPONSE_IP, msg);
			//cout << "1111" << endl;
			SendMessage(singleTLV);
			break;
		}
		
		case GameMsgType::GAME_MSG_UPDATEGAMESERVER_INFO:
		{
			
			auto msg = dynamic_cast<pb::GameServerInfo*>(single->mPbMsg);
			//add game server to serverlist
			//LoginServer::GetInstance().GetListener()->UpdateServerList(m_tcpFD, msg->ipaddress(), msg->portnum(), _responseLoad.curload(), _responseLoad.maxload());
			//add game server to consistent hash ring
			GamerServerInfo serverInfo;
			serverInfo.ip = msg->ipaddress();
			serverInfo.port = msg->portnum();
			serverInfo.curLoad = msg->curload();
			serverInfo.maxLoad = msg->maxload();
			LoginServer::GetInstance().GetListener()->UpdateServerList(serverInfo);


			// response
			auto backInfo = new pb::GameServerInfoBack;
			backInfo->set_succ(1);
			auto singleTLV = new GameSingleTLV(GameMsgType::GAME_MSG_UPDATEGAMESERVER_INFO_BACK, backInfo);
			SendMessage(singleTLV);			
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
		/*
		std::cout << "<----------------------------------------->" << std::endl;
		std::cout << "send to " << this->m_tcpFD << ":" << LoginServer::Convert2Printable(_output) << std::endl;
		std::cout << "<----------------------------------------->" << std::endl;
		*/
	}
	free(pOut);
	delete _singleTLV;
	_singleTLV = nullptr;
	
}
