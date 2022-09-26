#include "EventManager.hpp"

EventManager::EventManager() : m_shutdown(true)
{

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

void EventManager::eventLoop()
{
    std::unique_lock<std::mutex> evtLoopLock(m_mutex);

    while (!m_shutdown)
    {
        m_conditionVar.wait(evtLoopLock);
        while (!m_sender.eventQueueEmpty())
        {
            Event* evt = m_sender.nextEvent();
            // std::thread recvrInvokations(&EventManager::notifyReceivers, this, evt);
            // recvrInvokations.detach();
            notifyReceivers(evt);
            m_sender.dequeue();
        }
    }

    if (!m_shutdown){
        throw EventLoopException("Event loop has stopped unexpectedly!");
    }
}

void EventManager::start()
{
    try
    {
        m_shutdown = false;
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

void EventManager::notifyReceivers(Event* evt)
{
    while (!m_receiver.recevierQueueEmpty(evt->getEvtName()))
    {
        m_receiver.processAndDequeue(evt);
    }
    m_receiver.remove(evt);
}