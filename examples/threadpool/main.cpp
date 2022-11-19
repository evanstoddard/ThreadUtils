/*
 * Copyright (C) Evan Stoddard.
 */

/**
 * @file main.cpp
 * @author Evan Stoddard
 * @brief Threadpool Example
 */

#include <iostream>
#include <iomanip>
#include "runnable.h"
#include "threadpool.h"
#include <unistd.h>

/**
 * @brief Function to run in runnable
 *
 * @param n
 * @param index
 */
void runInRunnable(int index)
{
	sleep(1);
	std::cout << "Thread [0x" << std::hex << std::this_thread::get_id() << "]: Finished Runnable " << index << std::endl;
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

	//Read number of threads to run from command line
	int numThreads = ::atoi(argv[1]);
	if (numThreads < 1)
	{
		std::cout << "Invalid number of threads.  Must be > 0." << std::endl;
	}

	// Create threadpool object
	ThreadUtils::Threadpool threadpool(numThreads);

	// Create runnable objects for example
	for (int i = 0; i < 10; i++)
	{
		// Create runnable
		threadpool.enqueue_new(runInRunnable, i);
	}
	threadpool.start();

	// Create runnable to notify us when we're done
	ThreadUtils::Runnable<> *finalStage = new ThreadUtils::Runnable<>([&]() {
		gFinishedConditionVariable.notify_one();
	});
	threadpool.enqueue(finalStage);

	// Wait for thread pool to complete
	std::unique_lock<std::mutex> l(gFinishedMutex);
	gFinishedConditionVariable.wait(l);

	// Stop threadpool
	threadpool.stop();

	return 0;
}