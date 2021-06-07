The purpose of this project is to validate an Intel/AMD processor feature that
guarantees integer read/write atomicity. Having spent years leveraging
such as the interlocked* functions in Windows (and their
Linux equivalents), needless to say it came as quite a shock to me, so much so
that, probably like you, I didn't believe it.  Before you call the Inquisition
on me, the behaviour I'm describing is documented in Volume 3a Part 1 of the
Intel 64 and IA-32 software developer manual, which can be found here:

https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.pdf


The relevant section is section 8.1.1, page 8-2:

	8.1.1         Guaranteed         Atomic         Operations
	The Intel486 processor (and newer processors since) guarantees
	that the following basic memory operations will always
	be carried out atomically:
	•Reading or writing a byte
	•Reading or writing a word aligned on a 16-bit boundary
	•Reading or writing a doubleword aligned on a 32-bit boundary
	The Pentium processor (and newer processors since) guarantees that the following additional memory operations will always be carried out atomically:
	•Reading or writing a quadword aligned on a 64-bit boundary
	•16-bit accesses to uncached memory locations that fit within a 32-bit data bus
	The P6 family processors (and newer processors since) guarantee that the following additional memory operation will always be carried out atomically:
	•Unaligned 16-, 32-, and 64-bit accesses to cached memory that fit within a cache line

This basically means that you are guaranteed that anytime you're reading from
or writing to a single memory location that's aligned on the relevant data
boundary, you are guaranteed that that read or write operation will be completed
successfully before another write can partially change the value. At the moment
you read from that memory location, your read will complete before another
thread can alter it. It's important to note that you can take a snapshot of a
memory location by copying it elsewhere or update it singularly, but there's
if you need further synchronization you need to use other synchronization
techniques. In other words, just because I can read it now and examine or copy
out its value, does not mean that that value won't change a nanosecond after
I read it. What's guaranteed is that it won't be changed while I'm in the
middle of reading it.

In any case, I thought, "I want to see this in action."  So I wrote this test
My testing scenario is basically the following: Spawn an equal number of threads
that are all trying to simultaneously read from or write to a single memory
location. I've chosen a handful of "magic numbers" that will be randomly written
to the 64 bit variable, and under no circumstances can the single memory
location deviate from one of the magic numbers. That is to say, each
iteration in each writer thread is randomly choosing one of the magic numbers and
writing to the single, unprotected, 64 bit unsigned integer.

Simultaneously, each iteration of each reader thread is examining the value in
the same 64 bit unsigned integer and comparing the value that was just read from
to the collection of magic values to ensure make that the value that was read from
the global is indeed one of the magic values. If in any case the value read is
not one of the magic values, we know that one of the writes to the memory address
is corrupted, meaning it didn't complete successfully before one of the other
threads simultaneously tried to write to that memory location started writing
a new value, representing a data integrity issue.


I compiled the code and ran it on Linux with g++ 10.2.1, Windows with
MSVC version 19.29.30037, and Windows with C++ Builder version 7.4 (bcc32c,
which is based off an older port of Clang. With the exception of
Visual C++ (which seems to have a bug related to searching a std::set; more on
that in a future article), it behaved as predicted, except for when
it didn't. More on that in a moment.

Satisfied-ish with the results, I added high-resolution timing information
to the code to observe some timings and then compare them to what they would
be if I used std::atomic.  You should absolutely use std::atomic anyway and not
rely on the processor's built-in guaranteed atomicity, especially at a time when
the ubiquitousness of the IA-32/x64 instruction set is rapidly becoming less of
a given. ARM and RISC5 are either here or on the way, depending on your use case,
and you would have to assume that they offer the same level (or any level) of
atomicity guarantees as AMD/Intel. But you should use std::atomic for not just
for processor compatibility reasons- it turns out you should also use it for
speed reasons.

When I modified the program to use std::atomic, the performance of the program
actually increased slightly. I tried this on both Linux and Windows and the result
was the same.

So the moral of the story:
1. use std::atomic for performance reasons
2. use std::atomic for compatibility reasons
3. If for some reason in the past you didn't use one of these and you had a
	flag or some other integer-derived value, your stuff would have worked
	anyway.


## Building
I included a very simple makefile for g++; simply type "make" in the directory where
the source code is. For Visual Studio or Borland C++, I recommend you create a
solution (VS) or a project (Borland C++) from the IDE and add the single .cpp
file, thr_test.cpp. It requires C++ 17 compilation, and make sure the directory
that contains the src and header files is in the include path.

By default it ships with not using std::atomic, though you can enable the
use of std::atomic by commenting out the #define USE_ATOMIC entry in main.hpp.





I'd like to thank the writers of the logging library I borrowed for this project
as well as the color library that the logging project uses. I'd also like to thank
Go for the remarkably simple and effective idea for a WaitGroup from sync.WaitGroup,
which I humbly provide a poor-man's C++ implementation below.

Logging Project
https://gist.github.com/kevinkreiser/39f2e39273c625d96790

Colorized console output for Windows
https://github.com/imfl/color-console

Go-like WaitGroup for C++
https://gist.github.com/SlowPokeInTexas/4c9c9b5ac6418d089944bb8fa3a1a60b


