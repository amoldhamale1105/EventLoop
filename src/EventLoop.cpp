#include "EventLoop.hpp"

EventManager EventLoop::m_evtManager;

void EventLoop::Run()
{
	if (!m_evtManager.isRunning())
		m_evtManager.start();
}

void EventLoop::Halt()
{
	if (m_evtManager.isRunning())
		m_evtManager.stop();
}

void EventLoop::RegisterEvent(const std::string& evtName, const std::function<void(Event*)>& callback)
{
	if (evtName.empty()){
		std::cerr<<"Invalid event name. Failed to register event"<<std::endl;
		return;
	}
	m_evtManager.registerCallback(evtName, callback);
}

void EventLoop::RegisterEvents(const std::vector<std::string>& events, const std::function<void(Event*)>& callback)
{
	for(const std::string& evtName : events)
	{
		RegisterEvent(evtName, callback);
	}
}

void EventLoop::DeregisterEvent(const std::string& evtName)
{
	if (evtName.empty()){
		std::cerr<<"Invalid event name. Failed to deregister event"<<std::endl;
		return;
	}
	m_evtManager.removeEvent(evtName);
}

void EventLoop::TriggerEvent(const std::string& evtName, void* data)
{
	if (evtName.empty()){
		std::cerr<<"Invalid event name. Failed to trigger event"<<std::endl;
		return;
	}
	Event* evt = new Event(evtName, data);
	m_evtManager.processEvent(evt);
}

void EventLoop::TriggerEvent(const std::string& evtName, const size_t& timeoutMS, void* data)
{
	if (evtName.empty()){
		std::cerr<<"Invalid event name. Failed to trigger event"<<std::endl;
		return;
	}
	if (timeoutMS == 0){
		TriggerEvent(evtName, data);
		return;
	}

	Event* evt = new Event(evtName, data);
	std::chrono::time_point<std::chrono::system_clock> currentTime = std::chrono::system_clock::now();
	std::chrono::time_point<std::chrono::system_clock> wakeupTime = currentTime + std::chrono::milliseconds(timeoutMS);
	m_evtManager.scheduleEvent(evt, wakeupTime);
}