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
    void registerCallback(const std::string& evtName, const std::function<void(Event*)>& callback);
    void processEvent(Event* evt);
    void removeEvent(const std::string& evtName);
    
    void start();
    void stop();
    bool isRunning() const;

private:
    void eventLoop();
    
    bool m_shutdown;
    std::mutex m_mutex;
    std::condition_variable m_conditionVar;

    EventSender m_sender;
    EventReceiver m_receiver;
};