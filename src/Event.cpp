#include "Event.hpp"

Event::Event(const std::string& evtName) : m_evtName(evtName), m_data(nullptr)
{

}

Event::Event(const std::string& evtName, void* data) : m_evtName(evtName), m_data(data)
{

}

Event::~Event()
{
    // DO NOT use this destructor to delete the m_data pointer. That data is not owned or created by us
}

std::string Event::getEvtName() const
{
    return m_evtName;
}

void* Event::getData() const
{
    return m_data;
}