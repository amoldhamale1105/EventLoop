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

Event* EventSender::nextEvent() const
{
    return m_evtQueue.front();
}

bool EventSender::eventQueueEmpty() const
{
    return m_evtQueue.empty();
}