/*
 * Copyright (C) Evan Stoddard.
 */

/**
 * @file main.cpp
 * @author Evan Stoddard
 * @brief Buffered Threadpool Example
 */

#include <iostream>
#include <iomanip>
#include "runnable.hpp"
#include "orderedbufferedthreadpool.hpp"
#include <chrono>
#include <thread>
#include <unistd.h>
#include <deque>

ThreadUtils::OrderedBufferedThreadpool<std::string, int> *threadpool;

void threadFunc(int delay, const std::string val, int tag, bool valid)
{
	// Delay
	std::this_thread::sleep_for(std::chrono::seconds(delay));

	// Complete thread
	if (!valid)
	{
		threadpool->invalidateTag(tag);
	}
	else
	{
		threadpool->feedOutputQueue(val, tag);
	}

}

/**
 * @brief Entry point
 *
 * @param argc Num args
 * @param argv Args list
 * @return int Return code
 */
int main(int argc, char **argv)
{
	// Check number of arguments
	if (argc < 2)
	{
		std::cout << "Please enter number of threads to use." << std::endl;
		return 1;
	}

	// Read number of threads to run from command line
	int numThreads = ::atoi(argv[1]);
	if (numThreads < 1)
	{
		std::cout << "Invalid number of threads.  Must be > 0." << std::endl;
	}

	// Create threadpool
	threadpool = new ThreadUtils::OrderedBufferedThreadpool<std::string, int>(numThreads);

	using namespace ThreadUtils;

	int delay[] = {5, 3, 4, 1};
	std::string val[] = {"This", "is", "NOT", "awesome!"};
	bool valid[] = {true, true, false, true};

	// Start threadpool
	threadpool->start();

	// Feed input queue
	for (int i = 0; i < 4; i++)
	{
		threadpool->feedQueue(new Runnable<int, const std::string, int, bool>
		(
			threadFunc,
			delay[i],
			val[i],
			i,
			valid[i]
		), i);
	}

	while(true)
	{
		std::string val = threadpool->fetchFromBuffer();
		std::cout << "Val: " << val << std::endl;

		if (val == "awesome!")
		{
			threadpool->stop();
			break;
		}
	}

	return 0;
}