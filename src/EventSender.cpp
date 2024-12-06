/**
    C++ library providing the functionality of an event loop to handle 
    instantaneous and scheduled events in event driven system applications
    Copyright (C) 2022  Amol Dhamale

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
    USA
 */
#include "EventSender.hpp"

using EventLoop::Event;

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
