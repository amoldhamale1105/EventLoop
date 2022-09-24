#include <iostream>
#include <EventLoop.hpp>
#include <Common.h>

int main(int argc, char** argv)
{
	std::cout << getTime().c_str() << "Creating EventLoop" << std::endl;
    EventLoop<std::function<void()>>* eventloop = new EventLoop<std::function<void()>>(true);

    std::cout << getTime().c_str() << "Adding event element" << std::endl;
    eventloop->enqueue( 3000, []{ std::cout << getTime().c_str() << "Running task 3" << std::endl; } );
    eventloop->enqueue( 1000, []{ std::cout << getTime().c_str() << "Running task 1" << std::endl; } );
    eventloop->enqueue( 2000, []{ std::cout << getTime().c_str() << "Running task 2" << std::endl; } );

    std::this_thread::sleep_for( std::chrono::milliseconds(5000) );
    delete eventloop;
    std::cout << getTime().c_str() << "Exiting..." << std::endl;
    return 0;
}