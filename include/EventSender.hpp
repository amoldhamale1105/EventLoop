#pragma once

#include <queue>
#include <set>
#include <chrono>
#include <functional>
#include "Event.h"
#include "EventLoopException.hpp"

typedef std::pair<eventloop::Event*,std::chrono::time_point<std::chrono::system_clock>> EventSchedule;

class EventSender
{
public:
    EventSender();
    
    void enqueue(eventloop::Event* evt);
    void dequeue();
    void addScheduledEvent(eventloop::Event* evt, const std::chrono::time_point<std::chrono::system_clock>& wakeupTime);
    void removeEventSchedule();
    
    eventloop::Event* nextEvent() const;
    EventSchedule nextEventSchedule() const;
    
    bool eventQueueEmpty() const;
    bool eventScheduleEmpty() const;

private:
    class EventCompare
    {
    public:
        bool operator()(const EventSchedule& e1, const EventSchedule& e2) const
        {
            return e1.second < e2.second;
        }
    };
    std::queue<eventloop::Event*> m_evtQueue;
    std::multiset<EventSchedule, EventCompare> m_scheduledEvts;
};
