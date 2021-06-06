#pragma once


#include <thread>
#include <condition_variable>
#include <mutex>


namespace CppWaitGroup
{
	using	std::condition_variable;
	using	std::mutex;
	using	std::lock_guard;
	using	std::unique_lock;

	class WaitGroup
	{
	public:
		WaitGroup(){};

		void	Add( size_t count=1 )
		{
			lock_guard<mutex>	lock( _mutex );
			_waitCount	+=	count;
		}

		void    Done()
		{
			{
				lock_guard<mutex> lock( _mutex );
				_waitCount--;
			}
			_cond.notify_one();
		}

		void	Wait()
		{
			unique_lock<mutex>	lock(_mutex);

			_cond.wait( lock, [this]{ return _waitCount == 0; } );
		}

	protected:
		size_t				_waitCount=0;
		mutex				_mutex;
		condition_variable	_cond;
	};











};

