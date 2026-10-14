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
#include "EventReceiver.hpp"

using EventLoop::Event;

EventReceiver::EventReceiver()
{

}

void EventReceiver::enqueue(const std::string& evtName, const std::function<void(Event*)>& callback)
{
    std::lock_guard<std::mutex> rcvLock(m_mutex);
    auto evtPos = m_recvQueueMap.find(evtName);
    if (evtPos != m_recvQueueMap.end()){
        evtPos->second.push_back(callback);
    }
    else{
        std::list<std::function<void(Event*)>> callbackQueue;
        callbackQueue.push_back(callback);
        auto mapEntry = std::make_pair(evtName, callbackQueue);
        m_recvQueueMap.insert(mapEntry);
    }
}

void EventReceiver::notifyAndDequeue(Event* evt)
{
    std::unique_lock<std::mutex> rcvLock(m_mutex);
    auto evtPos = m_recvQueueMap.find(evt->getName());
    if (evtPos != m_recvQueueMap.end()){
        std::list<std::function<void(Event*)>> callbackQueue = evtPos->second;
        if (!callbackQueue.empty()){
            rcvLock.unlock();
            callbackQueue.front()(evt);
            rcvLock.lock();
            callbackQueue.pop_front();
        }
    }
}

void EventReceiver::notifyAllReceivers(Event* evt)
{
    std::unique_lock<std::mutex> rcvLock(m_mutex);
    auto evtPos = m_recvQueueMap.find(evt->getName());
    if (evtPos != m_recvQueueMap.end()){
        std::list<std::function<void(Event*)>> callbackQueue = evtPos->second;
        rcvLock.unlock();
        for(auto cbItr = callbackQueue.begin(); cbItr != callbackQueue.end(); ++cbItr)
        {
            if (*cbItr != nullptr)
                (*cbItr)(evt);
        }
    }
}

void EventReceiver::remove(const std::string& evtName)
{
    std::lock_guard<std::mutex> rcvLock(m_mutex);
    auto evtPos = m_recvQueueMap.find(evtName);
    if (evtPos != m_recvQueueMap.end()){
        m_recvQueueMap.erase(evtPos);
    }
}

bool EventReceiver::recevierQueueEmpty(const std::string& evtName) const
{
    std::lock_guard<std::mutex> rcvLock(m_mutex);
    bool isEmpty{false};
    auto evtPos = m_recvQueueMap.find(evtName);
    if (evtPos != m_recvQueueMap.end()){
        isEmpty = evtPos->second.empty();
    }
    else{
        isEmpty = true;
    }
    return isEmpty;
}
