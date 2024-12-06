#pragma once

#include <unordered_map>
#include <list>
#include <functional>
#include "Event.h"

class EventReceiver
{
public:
    EventReceiver();
    void enqueue(const std::string& evtName, const std::function<void(EventLoop::Event*)>& callback);
    void notifyAndDequeue(EventLoop::Event* evt);
    void notifyAllReceivers(EventLoop::Event* evt);
    void remove(const std::string& evtName);
    bool recevierQueueEmpty(const std::string& evtName) const;

private:
    std::unordered_map<std::string, std::list<std::function<void(EventLoop::Event*)>>> m_recvQueueMap;
};
