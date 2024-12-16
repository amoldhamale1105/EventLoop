/**
 * @file EventLoop.hpp
 * @author Amol Dhamale (amoldhamale1105@github.com)
 * @brief C++ event loop to handle instantaneous and scheduled events for event driven systems. This event loop can work independently by blocking
 * the main thread or in conjunction with other event loops by running on a secondary thread. Use this header and link libEventLoop.so in your project
 * @version 1.0
 * @date 2022-10-03
 * 
 * @copyright Copyright (c) 2022 Amol Dhamale
 * 
 */
#pragma once

#include <vector>
#include <functional>
#include <string>

namespace EventLoop {

class Event;

enum Mode
{
    BLOCK,   /* default */
    NON_BLOCK
};
/**
* @brief Set the mode of event loop before event loop begins to run. Once running this call will take no effect
* 
* @param mode event loop mode as Mode::BLOCK or Mode::NON_BLOCK. 
* Default is BLOCK which blocks the main thread and starts event handling. Useful for custom GUI apps and other event driven software. 
* NON_BLOCK uses a secondary thread for the event handling without blocking the main thread. Can be used on top of existing blocking 
* event loop like Qt framework's exec()
*/
void SetMode(const Mode& mode);
    
/**
* @brief Start the event loop at the point of the placement of this call. Please note that if the event loop mode is BLOCK,
* code following this call will not execute until the loop execution is halted. NON_BLOCK mode will fall through on the current thread
*/
void Run();

/**
* @brief Stop the currently running event loop. This call is thread safe and will take care of resource cleanup. Residual events in the 
* queue will be discarded once the event loop halts so make sure no part of the software depends on a scheduled event after the termination
* of the event loop since such events will never be delivered to the corresponding receivers
*/
void Halt();
    
/**
* @brief Register an event callback handler for given event name. The registered handler must accept one argument as the Event* pointer
* 
* @param evtName name of the event to be subscribed for a notification
* @param callback callback to be invoked once the event is triggered with Event* arg delivering data and event name
*/
void RegisterEvent(const std::string& evtName, const std::function<void(Event*)>& callback);

/**
* @brief Register a common callback handler for multiple events. The registered handler must accept one argument as the Event* pointer
* 
* @param events collection of events to be subscribed for a notification
* @param callback callback to be invoked on trigger of any of the events in the collection with Event* arg delivering data and event name
* of respective event. The event name can be checked in common handlers to deal with specific events and their data
*/
void RegisterEvents(const std::vector<std::string>& events, const std::function<void(Event*)>& callback);

/**
* @brief Unregister or deregister an event from notifying any handlers. Please note that this action will prevent an event from notifying
* all the handlers registered for it.
* 
* @param evtName name of event which can be ignored for callback invokations
*/
void DeregisterEvent(const std::string& evtName);

/**
* @brief Trigger an instantaneous event to be delivered to all the receivers which have registered/subscribed to it. This will immediately 
* invoke all the callback handlers which have registered to this event along with associated data
* 
* @param evtName name of the event to be triggered to notify receivers who have registered to it
* @param data optional parameter containing address of the data to be delivered to the receiver. Any kind of data can be delivered with a void*,
* however care must be taken to typecast it correctly at the receiver end before dereferencing to prevent incorrect interpretation or 
* segmentation faults in case of invalid or NULL pointers
*/
void TriggerEvent(const std::string& evtName, void* data = nullptr);

/**
* @brief Trigger an event to be delivered to all the receivers after a specified timeout. This will schedule the event trigger 
* and invoke all the callback handlers as soon as the timer expires and deliver the event along with associated data
* 
* @param evtName name of the event to be triggered to notify receivers who have registered to it
* @param timeoutMS period in milliseconds after which the event should be delivered to receivers
* @param data optional parameter containing address of the data to be delivered to the receiver. Any kind of data can be delivered with a void*,
* however care must be taken to typecast it correctly at the receiver end before dereferencing to prevent incorrect interpretation or 
* segmentation faults in case of invalid or NULL pointers
*/
void TriggerEvent(const std::string& evtName, const size_t& timeoutMS, void* data = nullptr);

} // namespace EventLoop
