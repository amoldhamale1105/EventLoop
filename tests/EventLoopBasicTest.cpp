/**
 * @file EventLoopBasicTest.cpp
 * @brief Unit tests for the EventLoop public API and its internal helper classes
 *        (Event, EventLoopException, EventSender, EventReceiver, EventManager).
 *
 * Each test case has a short comment directly above it describing what it covers.
 *
 * IMPORTANT - test ordering:
 * EventManager is instantiated as a single static/global object (see the
 * `evtManager` instance in EventLoop.cpp), so every test case that touches the
 * event loop lifecycle shares that one instance and is order-dependent:
 *  - EventLoopLifecycleTest uses SetUpTestSuite() to start the loop once in
 *    NON_BLOCK mode; its test cases run in declaration order and the loop is
 *    halted by the last two of them.
 *  - blockMode_haltFromAnotherThread_test_scenario then runs the BLOCK-mode
 *    path and halts the loop again before returning.
 *  - staticDestructor_stopsRunningLoopAtProcessExit_test_scenario deliberately
 *    never halts the loop, so it must remain the LAST test in this file.
 */

#include <gtest/gtest.h>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <chrono>
#include <functional>

// Public API
#include <EventLoop.h>
#include <Event.h>

// Internal headers – unit tests are permitted to include these to reach
// private implementation details for thorough coverage.
#include <EventLoopException.hpp>
#include <EventReceiver.hpp>
#include <EventSender.hpp>

namespace {

// Polls `predicate` at 1ms intervals for up to `timeoutMs`, returning true as
// soon as it succeeds. Used instead of a single fixed sleep so tests fail fast
// on success and only pay the full timeout when something is actually wrong.
bool waitUntil(const std::function<bool()>& predicate, int timeoutMs)
{
    for (int waited = 0; waited < timeoutMs; ++waited)
    {
        if (predicate())
            return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return predicate();
}

}

// Covers EventLoop::Event's single-argument constructor.
TEST(EventTest, singleArgConstructor_test_scenario)
{
    EventLoop::Event e("singleArgEvent");
    EXPECT_EQ(e.getName(), "singleArgEvent");
    EXPECT_EQ(e.getData(), nullptr);
}

// Covers EventLoopException's constructor and what().
TEST(EventLoopExceptionTest, constructor_test_scenario)
{
    EventLoopException ex("test error message");
    EXPECT_STREQ(ex.what(), "test error message");
}

// Covers EventSender::nextEventSchedule() throwing when the schedule is empty.
TEST(EventSenderTest, nextEventSchedule_throwsOnEmptySchedule_test_scenario)
{
    EventSender sender;
    EXPECT_TRUE(sender.eventScheduleEmpty());
    EXPECT_THROW(sender.nextEventSchedule(), EventLoopException);
}

// Fixture owning the two scheduled Event objects used by the ordering test below.
class EventSenderSchedulingTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_event1 = new EventLoop::Event("scheduled1");
        m_event2 = new EventLoop::Event("scheduled2");
    }

    void TearDown() override
    {
        // Safety net only. The test itself takes ownership back from the
        // sender, deletes both events, and nulls these members out. They are
        // only still non-null here if the test body failed/returned early.
        delete m_event1;
        delete m_event2;
    }

    EventLoop::Event* m_event1 = nullptr;
    EventLoop::Event* m_event2 = nullptr;
};

// Covers EventCompare::operator(), invoked when the multiset backing the
// schedule compares two entries during insertion.
TEST_F(EventSenderSchedulingTest, addScheduledEvent_invokesEventCompare_test_scenario)
{
    EventSender sender;
    auto now = std::chrono::system_clock::now();

    sender.addScheduledEvent(m_event1, now + std::chrono::milliseconds(200));
    // Earlier timestamp forces a comparison against m_event1 during insertion.
    sender.addScheduledEvent(m_event2, now + std::chrono::milliseconds(50));
    EXPECT_FALSE(sender.eventScheduleEmpty());

    while (!sender.eventScheduleEmpty())
    {
        EventLoop::Event* evt = sender.nextEventSchedule().first;
        delete evt;
        sender.removeEventSchedule();
    }
    m_event1 = nullptr;
    m_event2 = nullptr;
    EXPECT_TRUE(sender.eventScheduleEmpty());
}

// Covers EventReceiver::recevierQueueEmpty() for both the absent-key branch
// and the present-key branch.
TEST(EventReceiverTest, recevierQueueEmpty_test_scenario)
{
    EventReceiver recv;

    EXPECT_TRUE(recv.recevierQueueEmpty("absent"));

    recv.enqueue("existingEvt", [](EventLoop::Event*) {});
    EXPECT_FALSE(recv.recevierQueueEmpty("existingEvt"));
}

// Covers EventReceiver::notifyAndDequeue() for a registered key (callback
// fires) and an unregistered key (silently ignored, must not crash).
TEST(EventReceiverTest, notifyAndDequeue_test_scenario)
{
    EventReceiver recv;
    bool called = false;
    recv.enqueue("myEvent", [&](EventLoop::Event*) { called = true; });

    EventLoop::Event e("myEvent");
    recv.notifyAndDequeue(&e);
    EXPECT_TRUE(called);

    EventLoop::Event unknown("unknown");
    recv.notifyAndDequeue(&unknown);
}

// Fixture for the NON_BLOCK event-loop lifecycle. The loop is started once
// for the whole suite since EventManager is a single global instance; each
// TEST_F below then registers/triggers its own uniquely-named event(s) and
// polls for the result, so the suite's tests stay independent of each other
// even though they share the running loop.
class EventLoopLifecycleTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        EventLoop::SetMode(EventLoop::NON_BLOCK);
        EventLoop::Run();
    }
};

// Covers the `if (evtName.empty())` guard on every public entry point that
// has one: each call must return early without registering/triggering anything.
TEST_F(EventLoopLifecycleTest, emptyEventNameGuards_returnEarlyWithoutAction_test_scenario)
{
    EventLoop::RegisterEvent("", [](EventLoop::Event*) {});
    EventLoop::DeregisterEvent("");
    EventLoop::TriggerEvent("");
    EventLoop::TriggerEvent("", static_cast<size_t>(100));

    // Give the loop a moment to prove it's still alive and unaffected.
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
}

// Covers TriggerEvent's `if (timeoutMS == 0)` branch, which falls through to
// the instant-trigger path instead of scheduling anything.
TEST_F(EventLoopLifecycleTest, zeroTimeoutTrigger_fallsThroughToInstantTrigger_test_scenario)
{
    std::atomic<bool> handled{false};
    EventLoop::RegisterEvent("ZeroTimeout", [&](EventLoop::Event*) { handled = true; });

    EventLoop::TriggerEvent("ZeroTimeout", static_cast<size_t>(0));

    EXPECT_TRUE(waitUntil([&] { return handled.load(); }, 200))
        << "ZeroTimeout handler was not invoked";
}

// Covers TriggerEvent(evtName, data): the data pointer passed at the trigger
// site must be the exact one the registered handler receives.
TEST_F(EventLoopLifecycleTest, dataEvent_deliversAssociatedData_test_scenario)
{
    std::atomic<bool> handled{false};
    int value = 42;

    EventLoop::RegisterEvent("DataEvent", [&](EventLoop::Event* evt) {
        EXPECT_EQ(evt->getName(), "DataEvent");
        EXPECT_EQ(*static_cast<int*>(evt->getData()), 42);
        handled = true;
    });

    EventLoop::TriggerEvent("DataEvent", &value);

    EXPECT_TRUE(waitUntil([&] { return handled.load(); }, 200))
        << "DataEvent handler was not invoked";
}

// Covers RegisterEvents(): one handler shared across multiple event names,
// invoked once per name triggered.
TEST_F(EventLoopLifecycleTest, registerEvents_sharedHandlerAcrossMultipleNames_test_scenario)
{
    std::atomic<int> count{0};
    EventLoop::RegisterEvents({"EvtA", "EvtB"}, [&](EventLoop::Event*) { count++; });

    EventLoop::TriggerEvent("EvtA");
    EventLoop::TriggerEvent("EvtB");

    ASSERT_TRUE(waitUntil([&] { return count.load() >= 2; }, 200))
        << "Only " << count.load() << "/2 handlers fired";
    EXPECT_EQ(count.load(), 2);
}

// Covers registering multiple independent handlers for the same event name:
// triggering it once must invoke all of them.
TEST_F(EventLoopLifecycleTest, multipleHandlers_bothInvokedForSameEvent_test_scenario)
{
    std::atomic<int> count{0};
    EventLoop::RegisterEvent("Multi", [&](EventLoop::Event*) { count++; });
    EventLoop::RegisterEvent("Multi", [&](EventLoop::Event*) { count++; });

    EventLoop::TriggerEvent("Multi");

    ASSERT_TRUE(waitUntil([&] { return count.load() >= 2; }, 200))
        << "Only " << count.load() << "/2 handlers fired";
    EXPECT_EQ(count.load(), 2);
}

// Covers DeregisterEvent(): a handler removed before the event is triggered
// must never be invoked.
TEST_F(EventLoopLifecycleTest, deregisterEvent_preventsHandlerInvocation_test_scenario)
{
    EventLoop::RegisterEvent("Removed", [&](EventLoop::Event*) {
        FAIL() << "DeregisterEvent failed: this handler should never be called";
    });
    EventLoop::DeregisterEvent("Removed");

    EventLoop::TriggerEvent("Removed");

    // Nothing to poll for: success means the FAIL() above never runs. Give the
    // loop a moment to have processed (and silently dropped) the event.
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

// Covers SetMode()/Run() being no-ops once the loop is already running: they
// must neither crash nor disrupt event delivery.
TEST_F(EventLoopLifecycleTest, noOpGuards_whileLoopIsRunning_test_scenario)
{
    std::atomic<bool> before{false}, after{false};
    EventLoop::RegisterEvent("BeforeNoOp", [&](EventLoop::Event*) { before = true; });
    EventLoop::RegisterEvent("AfterNoOp", [&](EventLoop::Event*) { after = true; });

    EventLoop::TriggerEvent("BeforeNoOp");
    ASSERT_TRUE(waitUntil([&] { return before.load(); }, 200));

    // Both must be no-ops here (isRunning() == true) - if they weren't, this
    // would attempt to relaunch the background threads mid-flight.
    EventLoop::SetMode(EventLoop::BLOCK);
    EventLoop::Run();

    EventLoop::TriggerEvent("AfterNoOp");
    EXPECT_TRUE(waitUntil([&] { return after.load(); }, 200))
        << "Loop stopped processing events after redundant SetMode()/Run() calls";
}

// Covers EventCompare::operator() through the public API: two scheduled
// events must fire in wakeup-time order regardless of trigger order.
TEST_F(EventLoopLifecycleTest, scheduledEvents_deliveredInWakeupOrder_test_scenario)
{
    std::mutex orderMutex;
    std::vector<std::string> order;

    EventLoop::RegisterEvent("Sched1", [&](EventLoop::Event*) {
        std::lock_guard<std::mutex> lock(orderMutex);
        order.push_back("Sched1");
    });
    EventLoop::RegisterEvent("Sched2", [&](EventLoop::Event*) {
        std::lock_guard<std::mutex> lock(orderMutex);
        order.push_back("Sched2");
    });

    // Sched1 has the longer delay; Sched2's earlier wakeup forces the
    // schedule to reorder them, so Sched2 must be delivered first.
    EventLoop::TriggerEvent("Sched1", static_cast<size_t>(60));
    EventLoop::TriggerEvent("Sched2", static_cast<size_t>(10));

    ASSERT_TRUE(waitUntil([&] {
        std::lock_guard<std::mutex> lock(orderMutex);
        return order.size() == 2;
    }, 300));

    std::lock_guard<std::mutex> lock(orderMutex);
    ASSERT_EQ(order.size(), 2u);
    EXPECT_EQ(order[0], "Sched2");
    EXPECT_EQ(order[1], "Sched1");
}

// Covers halting the loop from within a handler, and that the halt happens
// promptly relative to its scheduled delay, not just at some point before a
// long timeout.
TEST_F(EventLoopLifecycleTest, haltTiming_stopsLoopPromptly_test_scenario)
{
    std::atomic<bool> done{false};
    EventLoop::RegisterEvent("Stop", [&](EventLoop::Event*) {
        done = true;
        EventLoop::Halt();
    });

    constexpr long long delayMs = 50;
    auto start = std::chrono::steady_clock::now();
    EventLoop::TriggerEvent("Stop", static_cast<size_t>(delayMs));

    ASSERT_TRUE(waitUntil([&] { return done.load(); }, 500))
        << "Event loop did not halt within 500 ms";

    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();

    EXPECT_GE(elapsedMs, delayMs)
        << "Stop fired earlier than its scheduled " << delayMs << " ms delay";
    EXPECT_LT(elapsedMs, delayMs + 150)
        << "Stop took " << elapsedMs << " ms, well beyond its " << delayMs << " ms delay";

    // Allow the background threads (m_mainLoop, m_scheduler) to finish before
    // the next test in this suite touches EventManager again.
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
}

// Runs immediately after the loop has been halted above. Leaves an instant
// event (lands in m_sender's queue but is never dispatched) and a long-delay
// scheduled event (left in m_scheduledEvts since the scheduler thread has
// already exited) so the EventManager destructor's unconditional queue/
// schedule cleanup loops have something to clean up when the process exits.
TEST_F(EventLoopLifecycleTest, postHalt_leavesEntriesForDestructorCleanup_test_scenario)
{
    EventLoop::RegisterEvent("PostHalt", [](EventLoop::Event*) {});
    EventLoop::TriggerEvent("PostHalt");
    EventLoop::TriggerEvent("PostHalt", static_cast<size_t>(120000));
}

// Covers the BLOCK-mode branch of EventManager::start(): eventLoop() runs
// synchronously on the calling thread, so Run() only returns once Halt() is
// called from elsewhere.
TEST(EventLoopTest, blockMode_haltFromAnotherThread_test_scenario)
{
    std::atomic<bool> eventHandled{false};

    EventLoop::SetMode(EventLoop::BLOCK);
    EventLoop::RegisterEvent("BlockModeEvent", [&](EventLoop::Event*) { eventHandled = true; });

    std::thread stopper([&] {
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        EventLoop::TriggerEvent("BlockModeEvent");
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        EventLoop::Halt();
    });

    EventLoop::Run(); // blocks here until the stopper thread calls Halt()
    stopper.join();

    EXPECT_TRUE(eventHandled.load());
}

// Covers the EventManager destructor path taken when the loop is still
// running at process exit: m_shutdown is false, so the destructor itself
// calls stop() and joins the background thread.
//
// This test intentionally never calls EventLoop::Halt(), leaving the static
// EventManager instance running until the test binary exits. It must remain
// the LAST test in this file - see the ordering note at the top.
TEST(EventLoopTest, staticDestructor_stopsRunningLoopAtProcessExit_test_scenario)
{
    EventLoop::SetMode(EventLoop::NON_BLOCK);
    EventLoop::RegisterEvent("NeverHalted", [](EventLoop::Event*) {});
    EventLoop::Run();
    EventLoop::TriggerEvent("NeverHalted");

    // Give the background thread a moment to start looping before the
    // process exits and the static destructor takes over cleanup.
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
}