#include <iostream>
#include <EventLoop.hpp>
#include <Common.h>
#include <unistd.h>

void sender();
void handler(Event*);

int main(int argc, char** argv)
{
    EventLoop::RegisterEvents({"test0","test1","test7","test14","test17"}, &handler);
    std::thread sendThread(&sender);
	std::cout <<"Setting up event loop" << std::endl;
    EventLoop::Run();
    std::cout<<"Ended event loop and waiting for thread to finish"<<std::endl;
    sendThread.join();
    std::cout<<"Exiting.."<<std::endl;
    
    return 0;
}

void sender()
{
    for(int i = 0; i < 20; i++)
    {
        if (i == 15){
            EventLoop::Halt();
        }
        EventLoop::TriggerEvent("test"+std::to_string(i), (void*)&i);
        usleep(1000000);
    }   
}

void handler(Event* evt)
{
    int* data = static_cast<int*>(evt->getData());
    std::cout<<"handled received event "<<evt->getEvtName()<<" with data: "<<*data<<std::endl;
}