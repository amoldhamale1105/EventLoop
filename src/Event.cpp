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

std::string Event::getName() const
{
    return m_evtName;
}

void* Event::getData() const
{
    return m_data;
}