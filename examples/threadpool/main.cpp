/*
 * Copyright (C) Evan Stoddard.
 */

/**
 * @file main.cpp
 * @author Evan Stoddard
 * @brief Threadpool Example
 */

#include <iostream>
#include "runnable.h"
#include "threadpool.h"
#include <chrono>
#include <thread>
#include <unistd.h>
#include <deque>

void runner(int32_t n, int index)
{
	std::cout << "Thread [" << std::this_thread::get_id() << "]: Sleeping for: " << index << " seconds." << std::endl;
	sleep(index);
	std::cout << "Thread [" << std::this_thread::get_id() << "]: Finished " << index << std::endl;
}

std::mutex gFinishedMutex;
std::condition_variable gFinishedConditionVariable;

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

	// Create threadpool object
	ThreadUtils::Threadpool threadpool(numThreads);

	// Create runnable objects for example
	for (int64_t i = 0; i < 10; i++)
	{
		// Create runnable
		ThreadUtils::Runnable<void(int64_t, int)> *runnable = new ThreadUtils::Runnable<void(int64_t, int)>(runner, rand(), i);
		threadpool.push(runnable);
	}
	threadpool.start();

	// Create runnable to notify us when we're done
	ThreadUtils::Runnable<void(void)> *finalStage = new ThreadUtils::Runnable<void(void)>([&]() {
		gFinishedConditionVariable.notify_one();
	});
	threadpool.push(finalStage);

	// Wait for thread pool to complete
	std::unique_lock<std::mutex> l(gFinishedMutex);
	gFinishedConditionVariable.wait(l);

	// Stop threads
	exit(0);

	return 0;
}