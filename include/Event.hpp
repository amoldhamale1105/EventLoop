#pragma once

#include <string>

class Event
{
public:
    Event(const std::string& evtName);
    Event(const std::string& evtName, void* data);
    virtual ~Event();

    std::string getEvtName() const;
    void* getData() const;

private:
    std::string m_evtName;
    void* m_data;
};