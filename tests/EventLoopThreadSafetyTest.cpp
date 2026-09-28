#include <gtest/gtest.h>
#include <EventLoop.h>
#include <Event.h>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

// TriggerEvent() from another thread while the loop runs on its own thread, as when events come
// from a library's callback threads.
TEST(EventLoopThreadSafetyTest, TriggerEventFromAnotherThread)
{
    std::mutex mutex;
    std::condition_variable condVar;
    bool handled = false;
    EventLoop::RegisterEvent("Ping", [&](EventLoop::Event*) {
        std::lock_guard<std::mutex> lock(mutex);
        handled = true;
        condVar.notify_one();
    });
    EventLoop::SetMode(EventLoop::Mode::NON_BLOCK);
    EventLoop::Run();

    std::thread producer([] { EventLoop::TriggerEvent("Ping"); });
    producer.join();

    {
        std::unique_lock<std::mutex> lock(mutex);
        EXPECT_TRUE(condVar.wait_for(lock, std::chrono::seconds(5), [&] { return handled; }));
    }
    EventLoop::Halt();
    EventLoop::DeregisterEvent("Ping");
}
