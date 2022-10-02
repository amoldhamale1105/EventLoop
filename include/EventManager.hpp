#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <functional>
#include "EventSender.hpp"
#include "EventReceiver.hpp"
#include "EventLoopException.hpp"

class EventManager
{
public:
    EventManager();
    ~EventManager();
    
    void registerCallback(const std::string& evtName, const std::function<void(Event*)>& callback);
    void processEvent(Event* evt);
    void scheduleEvent(Event* evt, const std::chrono::time_point<std::chrono::system_clock>& wakeupTime);
    void removeEvent(const std::string& evtName);
    
    void start();
    void stop();
    bool isRunning() const;

private:
    void eventLoop();
    void eventScheduler();
    void processScheduledEvents();
    
    bool m_shutdown, m_haltScheduler;
    std::thread m_scheduler;
    std::mutex m_mutex, m_schMutex;
    std::condition_variable m_conditionVar, m_schCondVar;

    EventSender m_sender;
    EventReceiver m_receiver;
};