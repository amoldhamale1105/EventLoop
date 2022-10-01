#include "EventSender.hpp"

EventSender::EventSender()
{

}

void EventSender::enqueue(Event* evt)
{
    m_evtQueue.push(evt);
}

void EventSender::dequeue()
{
    if (!m_evtQueue.empty()){
        Event* evt = m_evtQueue.front();
        m_evtQueue.pop();
        delete evt;
    }
}

void EventSender::addScheduledEvent(Event* evt, const std::chrono::time_point<std::chrono::system_clock>& wakeupTime)
{
    m_scheduledEvts.insert(std::make_pair(evt, wakeupTime));
}

void EventSender::removeEventSchedule()
{
    if (!m_scheduledEvts.empty())
        m_scheduledEvts.erase(m_scheduledEvts.begin());
}

Event* EventSender::nextEvent() const
{
    return m_evtQueue.empty() ? nullptr : m_evtQueue.front();
}

EventSchedule EventSender::nextEventSchedule() const
{
    if (m_scheduledEvts.empty())
        throw EventLoopException("Invalid attempt of fetching from an empty set");
    
    return *m_scheduledEvts.begin();
}

bool EventSender::eventQueueEmpty() const
{
    return m_evtQueue.empty();
}

bool EventSender::eventScheduleEmpty() const
{
    return m_scheduledEvts.empty();
}