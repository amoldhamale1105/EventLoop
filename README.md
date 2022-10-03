# C++ Event Loop Provider
C++ library providing the functionality of an event loop to handle instantaneous and scheduled events in event driven system applications  
The event loop can be set up in any application and supports the following modes:
### 1. Blocking event loop `EventLoop::Mode::BLOCK`
This kind of event loop will block the current thread (usually main thread) where the `EventLoop::Run()` call has been issued  
The current thread will then continue processing events and delivering them to subscribed receivers as long as the loop is running  
With this functionality you can keep your app alive and keep processing events as long as the user session exists  
The blocking event loop has been optimally implemented to minimize cpu load using condition variables as opposed to a more simplistic implementation using an infintely polling `while` loop which can obviously be a poor strategy, maxing out the CPU being attention hungry all the time  
### 2. Non-blocking event loop `EventLoop::Mode::NON_BLOCK`
This configuration will run the event loop quietly on another thread but again in an optimized manner without simply mindlessly polling for new events as stated above  
This will prevent the current thread where the `EventLoop::Run()` has been issued from getting blocked  
Usually comes in handy where the current thread has to either get to some execution after starting the event loop on the same thread or if it uses a blocking event loop of its own. An example of the latter can be the Qt framework's `QGuiApplication::exec()` call which blocks the main thread and starts processing events  
A valid question to ask: *Why would I need a secondary event loop when a primary one like Qt's `exec()` already exists?*

### Why use EventLoop on top of a primary Qt event loop
I don't know about you but I badly missed some features in Qt's event loop system and the biggest was the ability to broadcast a signal to all receivers who have registered/subscribed to it. This was actually the primary motivating factor behind developing this event loop from scratch on my own. I could not find any suitable solutions on the internet and was also eager to create one on my own.
Broadcasting is very handy when you want multiple components of the application to act upon a single stimulus without having to connect with the sender explicitly for each of them. Good luck passing around sender instances to multiple recievers and creating a signal slot connection for each to enable reception of broadcast events in Qt! Passing and maintaining objects in C++ is not as easy as you think. This brings me to my next reason,  
EventLoop is a static class. No instantiation, scoping and memory management bullshit. Moreover, all methods used for the event loop are static offering an obvious advantage to the user. Issue a `EventLoop::TriggerEvent()`, `EventLoop::RegisterEvent()` or any call in the `EventLoop` class for that matter from absolutely any thread, anywhere in your code and the desired outcome can still be accomplished. No hassle of connecting to the sender to receive an event and vice versa. You just need to provide a suitable callback to the `EventLoop::RegisterEvent()` call and you are good to go!

## Detailed Usage
As a user, you can dynamically link the event loop library to your application and include the `EventLoop.hpp` header in your application code to access the library methods. Other headers need to be present in your include path but should not used directly in your application as they are used by the `EventLoop` class as helper classes  
If directly used, the event loop might not work as expected and may cause problems. Thus it's highly recommended to use the event loop only via the `EventLoop.hpp` header file.  
To get started, here's a typical way to use the methods in the library for an event loop:  
- **[Optional step]** In the `main()` function of your program, call `EventLoop::SetMode()` method if you want the loop to be non-blocking. No need to call this explicitly for blocking mode because that's the default mode  
- **[Mandatory step]** Call the `EventLoop::Run()` method in `main()` where you wish the event loop to start and/or block  
- Register callbacks accepting `Event*` in any class or source file with `EventLoop::RegisterEvent()` where you wish to get notified for an event. Usually registrations are done in class constructors with either lambdas or class member functions as handlers  
- Trigger events anytime, anywhere in any thread of the application with `EventLoop::TriggerEvent()` and the corresponding handlers which registered for the particular event name will be invoked either instantenously or after a timeout depending on the type of overload used  
- Retrieve information from a received event in a registered handler like name and data using `Event` type's `Event::getName()` and `Event::getData()` methods  
- **[Optional but highly recommended step]** Enable the event loop to exit gracefully through the explicit call of `EventLoop::Halt()` method. This method needs to be called from a different thread if the mode is blocking.  

**Tip:** If you're planning to use `EventLoop` on top of Qt's `exec()` event loop, set up non-blocking `EventLoop::Run()` before the `exec()` call and `EventLoop::Halt()` to be invoked on the `QGuiApplication::aboutToQuit` signal as follows for a graceful exit
```
QObject::connect(&app, &QGuiApplication::aboutToQuit, []{ EventLoop::Halt(); });
```
**Note:** Any code just after a blocking `EventLoop::Run()` call on the same thread will obviously not execute because the `EventLoop::Run()` blocks the current thread to process events as per design.  
API description and parameters are detailed out in the `EventLoop.hpp` header and can be referred to for more details on using these methods  
In case of any questions or doubts, you can reach out to me at amoldhamale1105@gmail.com  

## Build instructions
Look out for latest stable releases of the library in the releases section of this repository but if you wish to build a library from source with the current source code version, then follow the below instructions depending on your dev environment
### VS Code
Open the project and click on the **CMake** option in the taskbar  
Configure the project using `Clean Reconfigure All`. Subsequently, build it using `Clean Rebuild All`  

### Terminal/shell
Open terminal window/shell and navigate to project root on the terminal  
Run the following comands once at the root of the project  
```
mkdir -p build
cd build
cmake ..
make
```
Build artifacts will be generated in the `build` directory  
Output artifact will be present in `lib` directory as `libEventLoop.so` which can be further linked or moved to your target destination  

## Contribution
I don't want to sound cliche but this was a hobby project conceived out of curiousity and exasperation of missing features in popular existing event loop implementations like mentioned above.  
Initially it seemed insignificant but when I saw the solution's immense potential in event driven systems wishing to use custom event loops, I decided to open it to the public domain for use, distribution and contribution. I have my own coding biases based on what I've learnt and I'm pretty sure many of the experts out there can help me improve my code with their valuable suggesstions and modifications for the benefit of everyone since this is open source software.  
Feel free to open pull requests, raise issues or contact me at amoldhamale1105@gmail.com for any modifications, suggestions or feature additions  
This library is licenced under LGPL v2.1 instead of any other fully permissive license because I care a lot about sharing improvements and making it available publicly through derivative work. I would love to see derivative and add-on work with this project and at the same time I don't mind people using it without modifications as a shared library linked to their own application. Hence the LGPL license decision in case any of you wondered.  