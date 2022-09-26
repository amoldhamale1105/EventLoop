#pragma once

#include <queue>
#include "Event.hpp"

class EventSender
{
public:
    EventSender();
    
    void enqueue(Event* evt);
    void dequeue();
    Event* nextEvent() const;
    bool eventQueueEmpty() const;

private:
    std::queue<Event*> m_evtQueue;
};