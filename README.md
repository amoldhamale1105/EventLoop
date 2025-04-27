# C++ Event Loop Provider
C++ library providing the functionality of an event loop to process instantaneous and scheduled events in event driven applications  

The event loop processes events to notify registered receivers optionally with data  

You can block the main thread for event monitoring or delegate it to a secondary thread leaving the main thread free for other tasks  
> **__Note:__** This library provides a framework for multi-threaded event processsing within the same process without any IPC capability

### Blocking event loop `EventLoop::Mode::BLOCK`
This mode of event loop will block the current thread (usually main thread) where the `EventLoop::Run()` call has been issued  
The current thread will then continue processing events as long as the loop is running   

### Non-blocking event loop `EventLoop::Mode::NON_BLOCK`
This mode will run the event loop in a secondary thread (created internally by the library) which will prevent the current thread where the `EventLoop::Run()` call has been issued from getting blocked  

This can prove useful where the current thread has to execute other routines after starting the event loop or defer to event loops defined by GUI frameworks for HMI applications. An example of the latter can be the Qt framework's `QGuiApplication::exec()` call which blocks the main thread and starts processing events  

### Value proposition 
- Simple string based events, making code more readable
- Global event trigger and event registration
- Dedicated thread for scheduled/deferred event handlers without dependency on caller thread
- Dedicated thread for instantaneous event handlers in non-blocking mode
- Optional data delivery with event
- Easy to integrate interface in existing codebase

## Detailed Usage
As a user, you can dynamically link the event loop library to your application and include the `EventLoop.h` header in your application code to access the library methods. Since we are using `Event` as a custom type to deliver and receive events with name and data, `Event.h` header will be required in the source files fetching those details from an incoming event  

> **Note:** DO NOT include any other headers from the *include* directory of this project apart from ones mentioned above when using the library in your application. Another way to remember is that you only need the `.h` header files for your app and all `.hpp` files can be ignored  

Detailed API documentation can be found in the `EventLoop.h` header. The following steps demonstrate possible usage of event loop in your application:  
- **[Optional step]** In the `main()` thread of your program, call `EventLoop::SetMode()` method if you want the loop to be non-blocking. No need to call this explicitly for blocking mode because that's the default mode  
- **[Mandatory step]** Call the `EventLoop::Run()` method in `main()` where you wish the event loop to start and/or block  
- Register callbacks accepting `EventLoop::Event*` in any class or source file with `EventLoop::RegisterEvent()` where you wish to get notified for an event. Usually registrations are done in class constructors with either lambdas or class member functions as handlers  
- Trigger events from any thread of the application with `EventLoop::TriggerEvent()` and the corresponding handlers which registered for the particular event name will be invoked either instantenously or after a timeout depending on the type of overload used  
- Retrieve information from a received event in a registered handler like name and data using `EventLoop::Event` type's `EventLoop::Event::getName()` and `EventLoop::Event::getData()` methods  
- **[Optional but highly recommended step]** Enable the event loop to exit gracefully through the explicit call of `EventLoop::Halt()` method. This method can be called from any thread in either modes. If called inside a registered handler, it will take effect only on completion of the handler scope  

> **Tip:** If you're planning to use `EventLoop` on top of Qt's `exec()` event loop, set up non-blocking `EventLoop::Run()` before the `exec()` call and `EventLoop::Halt()` to be invoked on the `QGuiApplication::aboutToQuit` signal as follows for a graceful exit
```
QObject::connect(&app, &QGuiApplication::aboutToQuit, []{ EventLoop::Halt(); });
```
> **Note:** Any code just after a blocking `EventLoop::Run()` call on the same thread will not execute until the event loop is halted because `EventLoop::Run()` blocks the current thread to process events by design.  

[Shopping cart](https://github.com/amoldhamale1105/ShoppingCart) app serves as a usage reference of this library and API. In case of any questions or clarifications, you can reach out to me at amoldhamale1105@gmail.com  

## Build instructions
Check for latest stable releases of the library under `Releases` but if you wish to build a library from source with the current source code version, run the `build.sh` script  

The script can be run without any options. Defaults for each option will appear in `{}` in the usage instruction. Learn about the script usage by running the following command
```
./build.sh -h
```
As an example, if you want to use the script to build for release mode with `Unix Makefiles` cmake generator, the script can be executed as follows
```
./build.sh -a -r -g "Unix Makefiles"
```
Build artifacts will be generated in the `build` directory  
Output artifact will be present in `lib` directory which can be further linked or moved to your target destination  

## Contribution
You are welcome to contribute to the project if you think 
- It's worth your time
- You can add some value
- You can fix a problem  

As a potential contributor you can 
- Review pull requests and [open issues](https://github.com/amoldhamale1105/EventLoop/issues) and check if you can resolve any of them
- Create [new issues](https://github.com/amoldhamale1105/EventLoop/issues/new) for bugs or feature requests so that either I or others in the community can get to it 
- Raise [PR](https://github.com/amoldhamale1105/EventLoop/pulls)s to address existing issues, to fix potential bugs or make any improvements to existing source code  

Feel free get in touch with me in case of any questions or suggestions amoldhamale1105@gmail.com