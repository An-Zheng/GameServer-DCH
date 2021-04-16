#pragma once
#include<map>
#include<string>
#include<string.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <mysql/mysql.h>
using namespace std;

#define LOGINSERVER_IP "192.168.18.128"
#define LOGINSERVER_PORT 8899
#define GAMESERVER_IP  "192.168.18.128"
#define GAMESERVER_PORT 10000
#define MAX_REPLICAS  1000


struct GamerServerInfo
{
	string	ip;
	int port;
	int curLoad;
	int maxLoad;
};

class PlayerInfo
{
    
public:
    PlayerInfo(string _playerName, int _tcpFd) :m_playerName(_playerName),m_tcpFd(_tcpFd) {}
    string m_playerName;
    int m_tcpFd;

};

class FnvHash
{
    static const unsigned int FNV_PRIME = 16777619u;
    static const unsigned int OFFSET_BASIS = 2166136261u;
    template <unsigned int N>
    static constexpr unsigned int fnvHashConst(const char (&str)[N], unsigned int I = N)
    {
        return I == 1 ? (OFFSET_BASIS ^ str[0]) * FNV_PRIME : (fnvHashConst(str, I - 1) ^ str[I - 1]) * FNV_PRIME;
    }
    static unsigned int fnvHash(const char* str)
    {
        const size_t length = strlen(str) + 1;
        unsigned int hash = OFFSET_BASIS;
        for (size_t i = 0; i < length; ++i)
        {
            hash ^= *str++;
            hash *= FNV_PRIME;
        }
        return hash;
    }
    struct Wrapper
    {
        Wrapper(const char* str) : str (str) { }
        const char* str;
    };
    unsigned int hash_value;
public:
    // calulate in run-time
    FnvHash(Wrapper wrapper) : hash_value(fnvHash(wrapper.str)) { }
    // calulate in compile-time
    template <unsigned int N>
    constexpr FnvHash(const char (&str)[N]) : hash_value(fnvHashConst(str)) { }
    // output result
    constexpr operator unsigned int() const { return this->hash_value; }
};
