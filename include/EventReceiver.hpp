#pragma once

#include <unordered_map>
#include <list>
#include <functional>
#include "Event.h"

class EventReceiver
{
public:
    EventReceiver();
    void enqueue(const std::string& evtName, const std::function<void(Event*)>& callback);
    void notifyAndDequeue(Event* evt);
    void notifyAllReceivers(Event* evt);
    void remove(const std::string& evtName);
    bool recevierQueueEmpty(const std::string& evtName) const;

private:
    std::unordered_map<std::string, std::list<std::function<void(Event*)>>> m_recvQueueMap;
};