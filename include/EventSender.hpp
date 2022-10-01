#pragma once

#include <queue>
#include <set>
#include <chrono>
#include <functional>
#include "Event.hpp"
#include "EventLoopException.hpp"

typedef std::pair<Event*,std::chrono::time_point<std::chrono::system_clock>> EventSchedule;

class EventSender
{
public:
    EventSender();
    
    void enqueue(Event* evt);
    void dequeue();
    void addScheduledEvent(Event* evt, const std::chrono::time_point<std::chrono::system_clock>& wakeupTime);
    void removeEventSchedule();
    
    Event* nextEvent() const;
    EventSchedule nextEventSchedule() const;
    
    bool eventQueueEmpty() const;
    bool eventScheduleEmpty() const;

private:
    class EventCompare
    {
    public:
        bool operator()(const EventSchedule& e1, const EventSchedule& e2)
        {
            return e1.second < e2.second;
        }
    };
    std::queue<Event*> m_evtQueue;
    std::multiset<EventSchedule, EventCompare> m_scheduledEvts;
};