#pragma once

#include <unordered_map>
#include <list>
#include <functional>
#include "Event.h"

class EventReceiver
{
public:
    EventReceiver();
    void enqueue(const std::string& evtName, const std::function<void(eventloop::Event*)>& callback);
    void notifyAndDequeue(eventloop::Event* evt);
    void notifyAllReceivers(eventloop::Event* evt);
    void remove(const std::string& evtName);
    bool recevierQueueEmpty(const std::string& evtName) const;

private:
    std::unordered_map<std::string, std::list<std::function<void(eventloop::Event*)>>> m_recvQueueMap;
};
