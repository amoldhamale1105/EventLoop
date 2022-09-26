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

void EventLoop::TriggerEvent(const std::string& evtName, void* data)
{
	if (evtName.empty()){
		std::cerr<<"Invalid event name. Failed to trigger event"<<std::endl;
		return;
	}
	if (m_evtManager.isRunning()){
		Event* evt = new Event(evtName, data);
		m_evtManager.processEvent(evt);
	}
	else{
		std::cerr<<"Failed to trigger event. Event loop has already stopped"<<std::endl;
	}
}