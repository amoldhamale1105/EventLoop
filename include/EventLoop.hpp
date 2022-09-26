#pragma once

#include "EventManager.hpp"
#include <vector>

class EventLoop
{
public:
    static void Run();
    static void Halt();
    
    static void RegisterEvent(const std::string& evtName, const std::function<void(Event*)>& callback);
    static void RegisterEvents(const std::vector<std::string>& events, const std::function<void(Event*)>& callback);
    static void DeregisterEvent(const std::string& evtName);
    static void TriggerEvent(const std::string& evtName, void* data = nullptr);

private:
    static EventManager m_evtManager;
};
