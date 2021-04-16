#include <iostream>
#include "LoginServer.h"
#include "tool.h"
using namespace std;
int main()
{
	/*
    short s = 1;
	if (s == htons(s))
		cout << "´ó¶Ë" << endl;
	 else 
		cout << "Ð¡¶Ë" << endl;
	*/
	
	auto listen = new TcpListener(PORT_NUM);
	LoginServer& loginServer = LoginServer::GetInstance();
	loginServer.Add_Channel(*listen);
	loginServer.AddListener(listen);
	loginServer.Run();
	

	return 0;
}