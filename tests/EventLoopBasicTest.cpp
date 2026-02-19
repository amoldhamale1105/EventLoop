#include <gtest/gtest.h>
#include <EventLoop.h>
#include <atomic>
#include <Event.h>

TEST(EventLoopTest, FullLifecycleCoverage)
{
    std::atomic<int> count{0};
    std::atomic<bool> delayedCalled{false};
    int value = 42;

    // RegisterEvent (basic)
    EventLoop::RegisterEvent("TestEvent", [&](EventLoop::Event*) {
        count++;
    });

    // Data + getName
    EventLoop::RegisterEvent("DataEvent", [&](EventLoop::Event* evt) {
        EXPECT_EQ(evt->getName(), "DataEvent");
        EXPECT_EQ(*static_cast<int*>(evt->getData()), 42);
        count++;
    });

    // RegisterEvents (vector)
    EventLoop::RegisterEvents({"A", "B"}, [&](EventLoop::Event*) {
        count++;
    });

    // Multiple handlers
    EventLoop::RegisterEvent("Multi", [&](EventLoop::Event*) { count++; });
    EventLoop::RegisterEvent("Multi", [&](EventLoop::Event*) { count++; });

    // DeregisterEvent
    EventLoop::RegisterEvent("X", [&](EventLoop::Event*) {
        FAIL() << "DeregisterEvent failed";
    });
    EventLoop::DeregisterEvent("X");

    // Delayed event (used to stop loop safely)
    EventLoop::RegisterEvent("Stop", [&](EventLoop::Event*) {
        delayedCalled = true;
        EventLoop::Halt();
    });

    // Trigger everything
    EventLoop::TriggerEvent("TestEvent");
    EventLoop::TriggerEvent("DataEvent", &value);
    EventLoop::TriggerEvent("A");
    EventLoop::TriggerEvent("B");
    EventLoop::TriggerEvent("Multi");
    EventLoop::TriggerEvent("X");            // should do nothing
    EventLoop::TriggerEvent("Stop", 10);     // delayed halt

    EventLoop::Run();

    EXPECT_TRUE(delayedCalled.load());
    EXPECT_GE(count.load(), 6);
}
