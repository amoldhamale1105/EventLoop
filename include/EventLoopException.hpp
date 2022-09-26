#pragma once

#include <iostream>

class EventLoopException : public std::runtime_error
{
public:
    EventLoopException(const char* error)
        : std::runtime_error(error)
    {
    }
};