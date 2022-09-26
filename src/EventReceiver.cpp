#include "EventReceiver.hpp"

EventReceiver::EventReceiver()
{

}

void EventReceiver::enqueue(const std::string& evtName, const std::function<void(Event*)>& callback)
{
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
    auto evtPos = m_recvQueueMap.find(evt->getEvtName());
    if (evtPos != m_recvQueueMap.end()){
        std::list<std::function<void(Event*)>> callbackQueue = evtPos->second;
        if (!callbackQueue.empty()){
            callbackQueue.front()(evt);
            callbackQueue.pop_front();
        }
    }
}

void EventReceiver::notifyAllReceivers(Event* evt)
{
    auto evtPos = m_recvQueueMap.find(evt->getEvtName());
    if (evtPos != m_recvQueueMap.end()){
        std::list<std::function<void(Event*)>> callbackQueue = evtPos->second;
        for(auto cbItr = callbackQueue.begin(); cbItr != callbackQueue.end(); ++cbItr)
        {
            (*cbItr)(evt);
        }
    }
}

void EventReceiver::remove(const std::string& evtName)
{
    auto evtPos = m_recvQueueMap.find(evtName);
    if (evtPos != m_recvQueueMap.end()){
        m_recvQueueMap.erase(evtPos);
    }
}

bool EventReceiver::recevierQueueEmpty(const std::string& evtName) const
{
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