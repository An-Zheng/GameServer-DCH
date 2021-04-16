#include "LoadBalancer.h"

int ConsistentHash::add(GamerServerInfo node)
{
    float loadRate = (float)node.curLoad / (float)node.maxLoad;
    int curReplicas = (1 - loadRate) * m_maxReplicas;

    for (int i = 0; i < curReplicas; i++)
    {
        string tmp = node.ip + to_string(node.port) + to_string(i);
        unsigned int hash = FnvHash(tmp.data());
        circle.insert(pair<unsigned int, GamerServerInfo>(hash, node));
    }
    return 0;
}

int ConsistentHash::remove(GamerServerInfo node)
{
    string tmp = node.ip + to_string(node.port) + to_string(0);
    unsigned int hash = FnvHash(tmp.data());
    auto  iter = circle.find(hash);
    if (iter != circle.end())
    {
        for (int i = 0; i < m_maxReplicas; i++)
        {
            tmp = node.ip + to_string(node.port) + to_string(i);
            hash = FnvHash(tmp.data());
            if (circle.find(hash) != circle.end())
            {
                circle.erase(hash);
            }

        }
    }

    return 0;
}

void ConsistentHash::update(GamerServerInfo node)
{
    cout << "------------PrintLoadInfo---------" << endl;

     cout << "server Ip " << node.ip << endl;
     cout << "server port " << node.port << endl;
     cout << "curLoad " << node.curLoad << endl;
     cout << "maxLoad " << node.maxLoad << endl;


    cout << "----------------------------------" << endl;

    remove(node);
    add(node);

}

GamerServerInfo ConsistentHash::get(string key)
{
    if (circle.empty())
    {
        return GamerServerInfo();
    }

    unsigned int hash = FnvHash(key.data());
    if (0 == circle.count(hash))
    {
        //map<int, Node>::iterator it_map = circle.upper_bound(hash);
        if (circle.upper_bound(hash) == circle.end())
        {
            return GamerServerInfo();
        }
        return circle.upper_bound(hash)->second;
    }
    return  GamerServerInfo();
}

