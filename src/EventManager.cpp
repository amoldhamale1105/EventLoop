#include "EventManager.hpp"

EventManager::EventManager() : m_shutdown(true)
{

}

EventManager::~EventManager()
{
    if (m_scheduler.joinable())
        m_scheduler.join();
}

void EventManager::registerCallback(const std::string& evtName, const std::function<void(Event*)>& callback)
{
    m_receiver.enqueue(evtName, callback);
}

void EventManager::processEvent(Event* evt)
{
    std::unique_lock<std::mutex> enqueLock(m_mutex);
    m_sender.enqueue(evt);
    m_conditionVar.notify_one();
}

void EventManager::scheduleEvent(Event* evt, const std::chrono::time_point<std::chrono::system_clock>& wakeupTime)
{
    std::unique_lock<std::mutex> schLock(m_schMutex);
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
        m_conditionVar.wait(evtLoopLock);
        while (!m_sender.eventQueueEmpty())
        {
            Event* evt = m_sender.nextEvent();
            m_receiver.notifyAllReceivers(evt);
            m_sender.dequeue();
        }
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
    while (!m_shutdown)
    {
        if (m_sender.eventScheduleEmpty())
            m_schCondVar.wait(schLoopLock);

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
        eventLoop();
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
        std::cerr << "An unknown exception occured in the event loop!" << '\n';
    }
}

void EventManager::stop()
{
    std::unique_lock<std::mutex> haltLock(m_mutex);
    m_shutdown = true;
    m_conditionVar.notify_all();
}

bool EventManager::isRunning() const
{
    return !m_shutdown;
}