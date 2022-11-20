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

ThreadUtils::OrderedBufferedThreadpool<int, int> *threadpool;

void threadFunc(int delay, int tag, bool valid)
{

	//std::cout << "Thread [0x" << std::hex << std::this_thread::get_id() << "]: "
	//		  << "Started processing tag " << tag << "." << std::endl;

	// Delay
	std::this_thread::sleep_for(std::chrono::seconds(delay));

	// Complete thread
	if (!valid)
	{
	//	std::cout << "Thread [0x" << std::hex << std::this_thread::get_id() << "]: "
	//		  << "Invalidated tag " << tag << "." << std::endl;
			  threadpool->invalidateTag(tag);
	}
	else
	{
		//std::cout << "Thread [0x" << std::hex << std::this_thread::get_id() << "]: "
		//	  << "Finished tag " << tag << "." << std::endl;
		threadpool->feedOutputQueue(tag, tag);
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
	threadpool = new ThreadUtils::OrderedBufferedThreadpool<int, int>(numThreads);

	using namespace ThreadUtils;

	for (int i = 0; i < 3; i++)
	{
		// Create runnable
		Runnable<int, int, bool> *runnable = new Runnable<int, int, bool>(threadFunc, 3 - i, i, i != 1);
		threadpool->feedQueue(runnable, i);
	}

	// Start threadpool
	threadpool->start();

	int sum = 0;
	while(true)
	{
		(void)threadpool->fetchFromBuffer();
	}

	return sum;
}