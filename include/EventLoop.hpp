#pragma once

#include "EventManager.hpp"
#include <vector>

enum Thread
{
    BLOCK = 0, //default
    NON_BLOCK        
};

class EventLoop
{
public:
    static void Run();
    static void Halt();
    
    static void RegisterEvent(const std::string& evtName, const std::function<void(Event*)>& callback);
    static void RegisterEvents(const std::vector<std::string>& events, const std::function<void(Event*)>& callback);
    static void DeregisterEvent(const std::string& evtName);

    static void TriggerEvent(const std::string& evtName, void* data = nullptr);
    static void TriggerEvent(const std::string& evtName, const size_t& timeoutMS, void* data = nullptr);

private:
    static EventManager m_evtManager;
};
