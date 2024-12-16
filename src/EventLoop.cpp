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
#include "EventLoop.h"
#include "EventManager.hpp"

static EventManager evtManager;

namespace EventLoop {

void Run()
{
	if (!evtManager.isRunning())
		evtManager.start();
}

void Halt()
{
	if (evtManager.isRunning())
		evtManager.stop();
}

void SetMode(const EventLoop::Mode& mode)
{
	if (!evtManager.isRunning())
		evtManager.blockPrimaryThread(mode == EventLoop::Mode::NON_BLOCK ? false : true);
}

void RegisterEvent(const std::string& evtName, const std::function<void(Event*)>& callback)
{
	if (evtName.empty()){
		std::cerr<<"Invalid event name. Failed to register event"<<std::endl;
		return;
	}
	evtManager.registerCallback(evtName, callback);
}

void RegisterEvents(const std::vector<std::string>& events, const std::function<void(Event*)>& callback)
{
	for(const std::string& evtName : events)
	{
		RegisterEvent(evtName, callback);
	}
}

void DeregisterEvent(const std::string& evtName)
{
	if (evtName.empty()){
		std::cerr<<"Invalid event name. Failed to deregister event"<<std::endl;
		return;
	}
	evtManager.removeEvent(evtName);
}

void TriggerEvent(const std::string& evtName, void* data)
{
	if (evtName.empty()){
		std::cerr<<"Invalid event name. Failed to trigger event"<<std::endl;
		return;
	}
	Event* evt = new Event(evtName, data);
	evtManager.processEvent(evt);
}

void TriggerEvent(const std::string& evtName, const size_t& timeoutMS, void* data)
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
	evtManager.scheduleEvent(evt, wakeupTime);
}

} // namespace EventLoop
