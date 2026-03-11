/**
 * @file EventLoopBasicTest.cpp
 * @brief Comprehensive unit tests targeting 100% function coverage and maximum line coverage.
 *
 * Coverage strategy:
 *  - Test 1 (DirectCoverage): exercises classes directly without the event-loop lifecycle.
 *    Covers Event single-arg constructor, EventLoopException constructor,
 *    EventReceiver::notifyAndDequeue / recevierQueueEmpty, EventSender throw path,
 *    and EventCompare::operator() via a local EventSender multiset insertion.
 *  - Test 2 (FullLifecycleCoverage): single NON_BLOCK lifecycle that covers all reachable
 *    runtime paths: SetMode + blockPrimaryThread, Halt no-op, all empty-name error branches,
 *    TriggerEvent with timeoutMS==0, two scheduled events (forces EventCompare ordering),
 *    SetMode / Run no-ops while running, and post-halt queue/schedule entries that trigger
 *    EventManager destructor cleanup (lines 34 and 38-41).
 *    NON_BLOCK mode also causes m_mainLoop to be joinable at program exit, covering the
 *    destructor join path (line 47).
 *
 * Known unreachable lines (defensive/dead code – cannot be covered by black-box tests):
 *  - EventManager::eventLoop(): throw after while(!m_shutdown) – dead code; the loop
 *    exits only when m_shutdown==true so !m_shutdown is always false there.
 *  - EventManager::start() and eventScheduler() catch blocks – only reachable if OS-level
 *    thread creation fails (resource exhaustion).
 *  - EventManager destructor line 45 (stop()) – requires the loop to be alive at program
 *    exit, which conflicts with safe test teardown.
 *  - EventManager line 149 (direct eventLoop() BLOCK path) – the NON_BLOCK path on
 *    line 151 is exercised instead.  Testing both modes in one binary is unsafe because
 *    the static EventManager singleton cannot be restarted after Halt() without joining
 *    the previous scheduler thread.
 */

#include <gtest/gtest.h>
#include <atomic>
#include <thread>
#include <chrono>

// Public API
#include <EventLoop.h>
#include <Event.h>

// Internal headers – unit tests are permitted to include these to reach
// private implementation details for thorough coverage.
#include <EventLoopException.hpp>
#include <EventReceiver.hpp>
#include <EventSender.hpp>

// =============================================================================
// Test 1: Direct class coverage (no event-loop lifecycle required)
// =============================================================================
TEST(EventLoopTest, DirectCoverage)
{
    // ── Event: single-argument constructor ──────────────────────────────────
    {
        EventLoop::Event e("singleArgEvent");
        EXPECT_EQ(e.getName(), "singleArgEvent");
        EXPECT_EQ(e.getData(), nullptr);
    }

    // ── EventLoopException: constructor ─────────────────────────────────────
    {
        EventLoopException ex("test error message");
        EXPECT_STREQ(ex.what(), "test error message");
    }

    // ── EventSender: nextEventSchedule() throw on empty schedule ────────────
    // This covers EventSender.cpp line 63 (throw path) as well as confirming
    // the EventLoopException constructor is exercised through a real throw.
    {
        EventSender sender;
        EXPECT_TRUE(sender.eventScheduleEmpty());
        EXPECT_THROW(sender.nextEventSchedule(), EventLoopException);
    }

    // ── EventSender: EventCompare::operator() via two multiset insertions ───
    // Inserting a second scheduled event with an earlier wakeup time causes
    // the multiset to invoke EventCompare::operator() to maintain ordering.
    {
        EventSender sender;
        auto now = std::chrono::system_clock::now();
        EventLoop::Event* e1 = new EventLoop::Event("scheduled1");
        EventLoop::Event* e2 = new EventLoop::Event("scheduled2");
        sender.addScheduledEvent(e1, now + std::chrono::milliseconds(200));
        // Earlier timestamp forces a comparison against e1 during insertion
        sender.addScheduledEvent(e2, now + std::chrono::milliseconds(50));
        EXPECT_FALSE(sender.eventScheduleEmpty());

        // Drain the queue to avoid leaks
        while (!sender.eventScheduleEmpty()) {
            EventLoop::Event* evt = sender.nextEventSchedule().first;
            delete evt;
            sender.removeEventSchedule();
        }
        EXPECT_TRUE(sender.eventScheduleEmpty());
    }

    // ── EventReceiver: recevierQueueEmpty – both branches ───────────────────
    {
        EventReceiver recv;

        // Else-branch: key absent → isEmpty = true
        EXPECT_TRUE(recv.recevierQueueEmpty("absent"));

        recv.enqueue("existingEvt", [](EventLoop::Event*) {});

        // If-branch: key present, non-empty list → isEmpty = false
        EXPECT_FALSE(recv.recevierQueueEmpty("existingEvt"));
    }

    // ── EventReceiver: notifyAndDequeue – key found and key absent ───────────
    {
        EventReceiver recv;
        bool called = false;
        recv.enqueue("myEvent", [&](EventLoop::Event*) { called = true; });

        // Key found, queue non-empty: callback is invoked
        EventLoop::Event e("myEvent");
        recv.notifyAndDequeue(&e);
        EXPECT_TRUE(called);

        // Key absent: no-op, must not crash
        EventLoop::Event unknown("unknown");
        recv.notifyAndDequeue(&unknown);
    }
}

// =============================================================================
// Test 2: Full lifecycle coverage (NON_BLOCK mode)
// =============================================================================
TEST(EventLoopTest, FullLifecycleCoverage)
{
    std::atomic<int>  count{0};
    std::atomic<bool> done{false};
    int value = 42;

    // ── Error paths: empty event name guards (before Run) ────────────────────
    // Each call hits the `if (evtName.empty())` true branch, prints to stderr,
    // and returns early – covering lines 49-50, 66-67, 75-76, 85-86 of EventLoop.cpp.
    EventLoop::RegisterEvent("", [](EventLoop::Event*) {});
    EventLoop::DeregisterEvent("");
    EventLoop::TriggerEvent("");
    EventLoop::TriggerEvent("", static_cast<size_t>(100));   // delayed overload, empty name

    // ── SetMode: covers EventLoop::SetMode() and EventManager::blockPrimaryThread()
    EventLoop::SetMode(EventLoop::NON_BLOCK);   // blockPrimaryThread(false)

    // ── Halt when not running: no-op guard (isRunning() == false) ────────────
    EventLoop::Halt();

    // ── TriggerEvent with timeoutMS == 0: falls through to instant TriggerEvent
    // (covers the `if (timeoutMS == 0)` true branch, lines 89-90 of EventLoop.cpp)
    EventLoop::RegisterEvent("ZeroTimeout", [&](EventLoop::Event*) { count++; });
    EventLoop::TriggerEvent("ZeroTimeout", static_cast<size_t>(0));

    // ── Register events for the main lifecycle ────────────────────────────────
    EventLoop::RegisterEvent("DataEvent", [&](EventLoop::Event* evt) {
        EXPECT_EQ(evt->getName(), "DataEvent");
        EXPECT_EQ(*static_cast<int*>(evt->getData()), 42);
        count++;
    });

    // RegisterEvents: common handler for multiple events
    EventLoop::RegisterEvents({"EvtA", "EvtB"}, [&](EventLoop::Event*) { count++; });

    // Multiple handlers on one event
    EventLoop::RegisterEvent("Multi", [&](EventLoop::Event*) { count++; });
    EventLoop::RegisterEvent("Multi", [&](EventLoop::Event*) { count++; });

    // Register then immediately deregister – handler must never fire
    EventLoop::RegisterEvent("Removed", [&](EventLoop::Event*) {
        FAIL() << "DeregisterEvent failed: this handler should never be called";
    });
    EventLoop::DeregisterEvent("Removed");

    // Handler that exercises the SetMode / Run no-op paths while running
    EventLoop::RegisterEvent("NoOpPaths", [&](EventLoop::Event*) {
        EventLoop::SetMode(EventLoop::BLOCK);  // no-op: loop is running
        EventLoop::Run();                      // no-op: loop is running
    });

    // Two scheduled events: second insertion forces EventCompare::operator()
    EventLoop::RegisterEvent("Sched1", [&](EventLoop::Event*) { count++; });
    EventLoop::RegisterEvent("Sched2", [&](EventLoop::Event*) { count++; });

    // Stop event: halts the loop when delivered
    EventLoop::RegisterEvent("Stop", [&](EventLoop::Event*) {
        done = true;
        EventLoop::Halt();
    });

    // ── Start the event loop (NON_BLOCK) ─────────────────────────────────────
    // EventManager line 151: m_mainLoop = std::thread(&EventManager::eventLoop, this)
    EventLoop::Run();

    // ── No-op Run / SetMode while loop is running ─────────────────────────────
    EventLoop::Run();                      // no-op: isRunning() == true
    EventLoop::SetMode(EventLoop::BLOCK);  // no-op: isRunning() == true

    // ── Trigger events ────────────────────────────────────────────────────────
    EventLoop::TriggerEvent("DataEvent", &value);
    EventLoop::TriggerEvent("EvtA");
    EventLoop::TriggerEvent("EvtB");
    EventLoop::TriggerEvent("Multi");
    EventLoop::TriggerEvent("Removed");       // deregistered → silently dropped
    EventLoop::TriggerEvent("NoOpPaths");

    // Two delayed events – second one exercises EventCompare via the public API
    EventLoop::TriggerEvent("Sched1", static_cast<size_t>(5));
    EventLoop::TriggerEvent("Sched2", static_cast<size_t>(10));

    // Schedule the halt
    EventLoop::TriggerEvent("Stop", static_cast<size_t>(50));

    // ── Wait for the Stop handler ─────────────────────────────────────────────
    for (int i = 0; i < 300 && !done.load(); ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    EXPECT_TRUE(done.load()) << "Event loop did not halt within 300 ms";

    // Allow background threads (m_mainLoop, m_scheduler) to finish
    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    // ── Post-halt: leave entries in queues for destructor cleanup coverage ────
    //
    // An instant event after Halt goes into m_sender's queue but is never
    // dispatched (loop stopped).  The static EventManager destructor will
    // call dequeue() on it at program exit (EventManager.cpp line 34).
    EventLoop::RegisterEvent("PostHalt", [](EventLoop::Event*) {});
    EventLoop::TriggerEvent("PostHalt");

    // A long-timeout scheduled event remains in m_scheduledEvts because the
    // scheduler thread has already exited.  The destructor will iterate and
    // delete it (EventManager.cpp lines 38-41).
    EventLoop::TriggerEvent("PostHalt", static_cast<size_t>(120'000));  // 2-minute timeout

    // ── Assertions ───────────────────────────────────────────────────────────
    // Minimum expected count:
    //   ZeroTimeout(1) + DataEvent(1) + EvtA(1) + EvtB(1)
    //   + Multi×2(2) + Sched1(1) + Sched2(1) = 8
    EXPECT_GE(count.load(), 8);
}