#pragma once
#include"tool.h"

using namespace std;

class ConsistentHash
{
public:
    ConsistentHash(int _maxReplicas) :m_maxReplicas(_maxReplicas) { }
    int add(GamerServerInfo node);


    int remove(GamerServerInfo node);

    void update(GamerServerInfo node);

    GamerServerInfo get(string key);


private:
    map<unsigned int, GamerServerInfo> circle;
    int m_maxReplicas;
};