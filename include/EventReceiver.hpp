#pragma once

#include <unordered_map>
#include <queue>
#include <functional>
#include "Event.hpp"

class EventReceiver
{
public:
    EventReceiver();
    void enqueue(const std::string& evtName, const std::function<void(Event*)>& callback);
    void processAndDequeue(Event* evt);
    void remove(Event* evt);
    bool recevierQueueEmpty(const std::string& evtName) const;

private:
    std::unordered_map<std::string, std::queue<std::function<void(Event*)>>> m_recvQueueMap;
};