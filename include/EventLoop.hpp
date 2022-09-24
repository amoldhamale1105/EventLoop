#include <stdio.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <set>
#include <functional>

#if defined( WIN32 )
    #include <windows.h>
#endif

class EventLoopNoElements : public std::runtime_error
{
public:
    EventLoopNoElements(const char* error)
        : std::runtime_error(error)
    {
    }
};

template <typename Type>
struct EventLoopCompare {
    typedef std::tuple<std::chrono::time_point<std::chrono::system_clock>, Type> TimePoint;

    bool operator()(const typename EventLoopCompare<Type>::TimePoint left, const typename EventLoopCompare<Type>::TimePoint right) {
        return std::get<0>(left) < std::get<0>(right);
    }
};

/**
 * You can enqueue any thing with this event loop. Just use lambda functions, future and promises!
 * With lambda `event.enqueue( 1000, [myvar, myfoo](){ myvar.something(myfoo); } )`
 * With futures we can get values from the event loop:
 * ```
 * std::promise<int> accumulate_promise;
 * event.enqueue( 2000, [&accumulate_promise](){ accumulate_promise.set_value(10); } );
 * std::future<int> accumulate_future = accumulate_promise.get_future();
 * accumulate_future.wait(); // It is not necessary to call wait, except for syncing the output.
 * std::cout << "result=" << std::flush << accumulate_future.get() << std::endl;
 * ```
 * It is just not a nice ideia to add something which hang the whole event loop queue.
 */
template <class Type>
class EventLoop {
    typedef std::multiset<
        typename EventLoopCompare<Type>::TimePoint,
        EventLoopCompare<Type>
    > EventLoopQueue;

    bool _shutdown;
    bool _free_shutdown;

    std::mutex _mutex;
    std::condition_variable _condition_variable;
    EventLoopQueue _queue;
    std::thread _runner;

public:
    // free_shutdown - if true, run all events on the queue before exiting
    EventLoop(bool free_shutdown);

    virtual ~EventLoop();

    // Add an element to the queue
    void enqueue(int timeout, Type element);

    // Mutex and condition variables are not movable and there is no need for smart pointers yet
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator =(const EventLoop&) = delete;
    EventLoop(const EventLoop&&) = delete;
    EventLoop& operator =(const EventLoop&&) = delete;

private:
    // To allow multiple threads to consume data, just add a mutex here and create multiple threads on the constructor
    void _event_loop();

    // Blocks until getting the first-element or throw EventLoopNoElements if it is shutting down
    // Throws EventLoopNoElements when it is shutting down and there are not more elements
    Type dequeue();
};

#define TEMPLATE_METHODS
#include "../src/EventLoop.cpp"