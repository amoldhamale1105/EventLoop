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
#include "EventManager.hpp"

uisng eventloop::Event;

EventManager::EventManager() : m_blockPrimary(true), m_shutdown(true), m_haltScheduler(false)
{

}

EventManager::~EventManager()
{
    while (!m_sender.eventQueueEmpty())
    {
        m_sender.dequeue();
    }
    while (!m_sender.eventScheduleEmpty())
    {
        Event* evt = m_sender.nextEventSchedule().first;
        if (evt != nullptr)
            delete evt;
        m_sender.removeEventSchedule();
    }
    
    if (!m_shutdown)
        stop();
    if (!m_blockPrimary && m_mainLoop.joinable())
        m_mainLoop.join();
    if (m_scheduler.joinable())
        m_scheduler.join();
}

void EventManager::blockPrimaryThread(bool blockPrimary)
{
    m_blockPrimary = blockPrimary;
}

void EventManager::registerCallback(const std::string& evtName, const std::function<void(Event*)>& callback)
{
    m_receiver.enqueue(evtName, callback);
}

void EventManager::processEvent(Event* evt)
{
    m_sender.enqueue(evt);
    m_conditionVar.notify_one();
}

void EventManager::scheduleEvent(Event* evt, const std::chrono::time_point<std::chrono::system_clock>& wakeupTime)
{
    m_sender.addScheduledEvent(evt, wakeupTime);
    m_schCondVar.notify_one();
}

void EventManager::removeEvent(const std::string& evtName)
{
    m_receiver.remove(evtName);
}

void EventManager::eventLoop()
{
    std::unique_lock<std::mutex> evtLoopLock(m_mutex);

    while (!m_shutdown)
    {
        while (!m_sender.eventQueueEmpty())
        {
            Event* evt = m_sender.nextEvent();
            m_receiver.notifyAllReceivers(evt);
            m_sender.dequeue();
        }
        m_conditionVar.wait(evtLoopLock, [&]{ return !m_sender.eventQueueEmpty() || m_shutdown; });
    }

    if (!m_shutdown){
        throw EventLoopException("Event loop has stopped unexpectedly!");
    }
}

void EventManager::eventScheduler()
{
    try
    {
        processScheduledEvents();
    }
    catch(const EventLoopException& e)
    {
        std::cerr << e.what() << '\n';
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    catch(...)
    {
        std::cerr << "An unknown exception occured in the event scheduler!" << '\n';
    }
}

void EventManager::processScheduledEvents()
{
    std::chrono::time_point<std::chrono::system_clock> currentTime, wakeupTime;

    std::unique_lock<std::mutex> schLoopLock(m_schMutex);
    while (!m_haltScheduler)
    {
        m_schCondVar.wait(schLoopLock, [&]{ return !m_sender.eventScheduleEmpty() || m_shutdown || m_haltScheduler; });
        if (m_shutdown || m_haltScheduler)
            break;

        wakeupTime = m_sender.nextEventSchedule().second;
        currentTime = std::chrono::system_clock::now();

        if (currentTime >= wakeupTime){
            processEvent(m_sender.nextEventSchedule().first);
            m_sender.removeEventSchedule();
            continue;
        }
        m_schCondVar.wait_until(schLoopLock, wakeupTime);
    }
}

void EventManager::start()
{
    try
    {
        m_shutdown = false;
        m_scheduler = std::thread(&EventManager::eventScheduler, this);
        if (m_blockPrimary)
            eventLoop();
        else
            m_mainLoop = std::thread(&EventManager::eventLoop, this);
    }
    catch(const EventLoopException& e)
    {
        std::cerr << e.what() << '\n';
        m_haltScheduler = true;
        m_schCondVar.notify_all();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        m_haltScheduler = true;
        m_schCondVar.notify_all();
    }
    catch(...)
    {
        std::cerr << "An unknown exception occured in the event loop!" << '\n';
        m_haltScheduler = true;
        m_schCondVar.notify_all();
    }
}

void EventManager::stop()
{
    m_shutdown = m_haltScheduler = true;
    m_conditionVar.notify_all();
    m_schCondVar.notify_all();
}

bool EventManager::isRunning() const
{
    return !m_shutdown;
}
