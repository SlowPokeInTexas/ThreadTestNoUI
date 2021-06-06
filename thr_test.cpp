#include "main.hpp"



//	This is the single memory location that all threads will read to and write from
#if	defined(USE_ATOMIC)
atomic<uint64_t>						TargetValue;
#else
uint64_t								TargetValue=0;
#endif

//	one of N values that will be written and tested for. We know that if TargetValue is any value other than
//	one of the 5 "magic" values listed below, then there was a problem with atomicity of writing to TargetValue
vector<uint64_t>    					MagicNumbers={19880205, 10661014, 19470708, 19200917, 00330403};
unordered_set<uint64_t>					SearchableMagicNumbers;

//	Timing unit definitions
typedef	std::chrono::nanoseconds		nanos;
typedef std::chrono::duration<float>	fsec;


//	Default to 50 million iterations
const uint64_t 							DEFAULT_ITERATIONS= 50000000;


//	Worket threads
void									writerThread( WaitGroup *wg, uint64_t iterationCount );
void									readerThread( WaitGroup *wg, uint64_t iterationCount );


int main( int argc, char *argv[] )
{
	uint64_t							iterations=DEFAULT_ITERATIONS;
	vector< shared_ptr<thread> >		allThreads;
	int									numberOfThreads=0;
	int									counter=0;


	WaitGroup							wg;


	try
	{
		//	Ask the OS how many cores it has, and then divide by 2 so we
		//	don't completely slam the system since we will be starting
		//	an equal number of reader and writer threads
		numberOfThreads		=	std::thread::hardware_concurrency() / 2;


		//	Check to see if they passed in their own iteration count
		if( argc > 1 )
		{
			char	*ptr=nullptr;

			iterations		=   strtoull( argv[1], &ptr, 10 );
		}

		//	Check to see if they passed in their own thread count
		if( argc > 2 )
		{
			numberOfThreads	=   atoi( argv[2] );
		}

		//	Sort the values for quick lookup
		for ( auto x : MagicNumbers)
		{
			SearchableMagicNumbers.insert( x );
		}


		logging::INFO(xsprintf("Launching %d Reader and %d Writer threads", numberOfThreads, numberOfThreads));

		//	Launch the threads
		for( counter=0; counter < numberOfThreads; counter++ )
		{
			auto reader= shared_ptr<thread>( new thread( readerThread, &wg, iterations ) );

			auto writer= shared_ptr<thread>( new thread( writerThread, &wg, iterations ) );

			allThreads.push_back( reader );
			allThreads.push_back( writer );

		}

	}
	catch( const exception &e )
	{
		logging::ERROR(xsprintf("Exception:%s", e.what()));
	}
	catch( ... )
	{
		logging::ERROR("Unknown exception encountered");
	}

	logging::INFO("Waiting for all threads to complete...");
	wg.Wait();


	//	Need to join our threads otherwise we get an abnormal termination
	std::for_each(allThreads.begin(), allThreads.end(), [](shared_ptr<thread>& t) { t->join(); });

	//	Clear out the thread objects
	allThreads.clear();

	return 0;
}


void writerThread( WaitGroup *wg, uint64_t iterationCount )
{
	std::mt19937						rng( std::chrono::steady_clock::now().time_since_epoch().count() );
	std::uniform_int_distribution<>		dist(0, 4);
	int									index=0;
	uint64_t							chosenValue=0;
	uint64_t							counter;
	high_resolution_clock::time_point	start;
	high_resolution_clock::time_point	end;

	try
	{
		wg->Add();

		logging::TRACE(xsprintf("Started Writer thread ID:%u, writing to the single global address %llu times", std::this_thread::get_id(), iterationCount));

		//	Since std::thread doesn't support a suspended launch mode (shame on you, std::thread),
		//	we'll initally sleep for 10 seconds which should be plenty enough time let the
		//	horses all out the gate
		std::this_thread::sleep_for(std::chrono::milliseconds(10000));

		start = high_resolution_clock::now();

		for( counter=0; counter < iterationCount; counter++ )
		{
			if( counter % 1000000 == 0 )
			{
				logging::TRACE(xsprintf("Writer thread %u has written %llu times",  std::this_thread::get_id(), counter ));
			}

			//	Get a random index
			index		=   dist(rng);

			//  Grab one of the magic values
			chosenValue	=   MagicNumbers[index];

			//	Write it to memory without locking
#if	defined(USE_ATOMIC)
			TargetValue.store( chosenValue );
#else
			TargetValue	=   chosenValue;
#endif
		}

		end = high_resolution_clock::now();
	}
	catch( const exception &e )
	{
		logging::ERROR(xsprintf("Exception:%s", e.what()));
	}
	catch( ... )
	{
		logging::ERROR("Unknown exception encountered");
	}


	fsec    totalTime = end - start;

	auto timeInNanoSeconds = std::chrono::duration_cast<nanos>(totalTime);

	stringstream	ss;

	ss << "Exiting Writer thread " << std::this_thread::get_id() <<  ", " << iterationCount << " iterations took " << timeInNanoSeconds.count() << "ns, averaging " << (timeInNanoSeconds.count() / iterationCount ) << "ns per iteration.";

	wg->Done();
	logging::INFO( ss.str() );
	return;
}
//---------------------------------------------------------------------------
void readerThread( WaitGroup *wg, uint64_t iterationCount )
{
	uint64_t							currentValue=0;
	uint64_t							counter;
	high_resolution_clock::time_point	start;
	high_resolution_clock::time_point	end;

#if !defined( _MSC_VER )
	unordered_set<uint64_t>				localMagicNumbers;
#else
	//	For some bizarro reason, a map in MSVC much faster than a set
	map<uint64_t, uint64_t>             localMagicNumbers;
#endif


	try
	{
		wg->Add();
		logging::TRACE(xsprintf("Started Reader thread ID:%u, reading from the single global address %llu times", std::this_thread::get_id(), iterationCount ));


		//	make a copy on each thread because occasionally I'd see weirdness
		//	with the iterator failing to find an element which was present.
#if defined(_MSC_VER )
		for( auto v : SearchableMagicNumbers )
		{
			localMagicNumbers[v]	=	v;
		}
		map<uint64_t,uint64_t>::iterator		endIterator;
		map<uint64_t, uint64_t>::iterator		it;

#else
		localMagicNumbers			=   SearchableMagicNumbers;
		std::unordered_set<uint64_t>::iterator	endIterator;
		std::unordered_set<uint64_t>::iterator	it;
#endif

		//	Since std::thread doesn't support starting in suspended mode, we'll sleep
		//	for 10 seconds which should approximately let the
		//	horses all out the gate roughly at the same time
		std::this_thread::sleep_for(std::chrono::milliseconds(10000));


		endIterator	=   localMagicNumbers.end();

		start		=	high_resolution_clock::now();
		for( counter=0; counter < iterationCount; counter++ )
		{
			if( counter % 1000000 == 0 )
			{
				logging::TRACE(xsprintf("Reader thread %u has read the global %llu times",  std::this_thread::get_id(), counter ));
			}


			//	Assign our global to our current value
#if	defined(USE_ATOMIC)
			currentValue	=   TargetValue.load();
#else
			currentValue	=	TargetValue;
#endif
			if( currentValue )
			{
				it	=	localMagicNumbers.find( currentValue );
				if( it == endIterator )
				{
					logging::ERROR(xsprintf("Value anomaly (%lld) detected in Reader thread ID:%u", currentValue, std::this_thread::get_id()));
				}
			}
		}	//	end for
		end		=	high_resolution_clock::now();
	}
	catch( const exception &e )
	{
		logging::ERROR(xsprintf("Exception:%s", e.what()));
	}

	catch( ... )
	{
		logging::ERROR("Unknown exception encountered");
	}
	fsec    totalTime = end - start;

	auto timeInNanoSeconds = std::chrono::duration_cast<nanos>(totalTime);

	stringstream	ss;

	ss << "Exiting Reader thread " << std::this_thread::get_id() <<  ", " << iterationCount << " iterations took " << timeInNanoSeconds.count() << "ns, averaging " << (timeInNanoSeconds.count() / iterationCount ) << "ns per iteration.";
	wg->Done();
	logging::INFO( ss.str() );
}
