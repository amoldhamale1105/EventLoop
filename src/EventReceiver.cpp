#include "EventReceiver.hpp"

EventReceiver::EventReceiver()
{

}

void EventReceiver::enqueue(const std::string& evtName, const std::function<void(Event*)>& callback)
{
    auto evtPos = m_recvQueueMap.find(evtName);
    if (evtPos != m_recvQueueMap.end()){
        evtPos->second.push(callback);
    }
    else{
        std::queue<std::function<void(Event*)>> callbackQueue;
        callbackQueue.push(callback);
        auto mapEntry = std::make_pair(evtName, callbackQueue);
        m_recvQueueMap.insert(mapEntry);
    }
}

void EventReceiver::processAndDequeue(Event* evt)
{
    auto evtPos = m_recvQueueMap.find(evt->getEvtName());
    if (evtPos != m_recvQueueMap.end()){
        if (!evtPos->second.empty()){
            evtPos->second.front()(evt);
            evtPos->second.pop();
        }
    }
}

void EventReceiver::remove(Event* evt)
{
    auto evtPos = m_recvQueueMap.find(evt->getEvtName());
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