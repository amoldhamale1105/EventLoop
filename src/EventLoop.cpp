#ifndef TEMPLATE_METHODS
#include <EventLoop.hpp>

#else

template <typename Type>
EventLoop<Type>::EventLoop(bool free_shutdown)
	: _shutdown(false),
	_free_shutdown(free_shutdown),
	_runner( &EventLoop<Type>::_event_loop, this )
{

}

template <typename Type>
EventLoop<Type>::~EventLoop()
{
	std::unique_lock<std::mutex> dequeuelock(_mutex);
	_shutdown = true;
	_condition_variable.notify_all();
	dequeuelock.unlock();

	if (_runner.joinable()) {
		_runner.join();
	}
}

template <typename Type>
void EventLoop<Type>::_event_loop()
{
	while ( true ) {
		try {
			Type call = dequeue();
			call();
		}
		catch (EventLoopNoElements&) {
			return;
		}
		catch (std::exception& error) {
			std::cerr << "Unexpected exception on EventLoop dequeue running: '" << error.what() << "'" << std::endl;
		}
		catch (...) {
			std::cerr << "Unexpected exception on EventLoop dequeue running." << std::endl;
		}
	}
	std::cerr << "The main EventLoop dequeue stopped running unexpectedly!" << std::endl;
}

template <typename Type>
void EventLoop<Type>::enqueue(int timeout, Type element)
{
	std::chrono::time_point<std::chrono::system_clock> timenow = std::chrono::system_clock::now();
	std::chrono::time_point<std::chrono::system_clock> newtime = timenow + std::chrono::milliseconds(timeout);

	std::unique_lock<std::mutex> dequeuelock(_mutex);
	_queue.insert(std::make_tuple(newtime, element));
	_condition_variable.notify_one();
}

template <typename Type>
Type EventLoop<Type>::dequeue()
{
	typename EventLoopQueue::iterator queuebegin;
	typename EventLoopQueue::iterator queueend;
	std::chrono::time_point<std::chrono::system_clock> sleeptime;

	// _mutex prevents multiple consumers from getting the same item or from missing the wake up
	std::unique_lock<std::mutex> dequeuelock(_mutex);
	do {
		queuebegin = _queue.begin();
		queueend = _queue.end();

		if ( queuebegin == queueend ) {
			if ( _shutdown ) {
				throw EventLoopNoElements( "There are no more elements on the queue because it already shutdown." );
			}
			_condition_variable.wait( dequeuelock );
		}
		else {
			if ( _shutdown ) {
				if (_free_shutdown) {
					break;
				}
				else {
					throw EventLoopNoElements( "The queue is shutting down." );
				}
			}
			std::chrono::time_point<std::chrono::system_clock> timenow = std::chrono::system_clock::now();
			sleeptime = std::get<0>( *queuebegin );
			if ( sleeptime <= timenow ) {
				break;
			}
			_condition_variable.wait_until( dequeuelock, sleeptime );
		}
	} while ( true );

	Type firstelement = std::get<1>( *queuebegin );
	_queue.erase( queuebegin );
	dequeuelock.unlock();
	return firstelement;
}

#endif