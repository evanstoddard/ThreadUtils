/*
 * Copyright (C) Evan Stoddard.
 */

/**
 * @file main.cpp
 * @author Evan Stoddard
 * @brief Buffered Threadpool Example
 */

#include <iostream>
#include "runnable.hpp"
#include "bufferedthreadpool.hpp"
#include <chrono>
#include <thread>
#include <unistd.h>
#include <deque>

ThreadUtils::BufferedThreadpool<int64_t> *threadpool;
void FirstStage(int64_t i);
void SecondStage(int64_t i);

void FirstStage(int64_t i)
{
	// Print stage and wait 1 second
	std::cout << "Stage 1: " << i << std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	// Create runnable for next stage
	threadpool->enqueue_new(SecondStage, i);
}

void SecondStage(int64_t i)
{
	// Print stage and wait 1 second
	std::cout << "Stage 2: " << i << std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	// Push to output buffer
	threadpool->feedOutputBuffer(i);
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

	// Create threadpool object
	threadpool = new ThreadUtils::BufferedThreadpool<int64_t>(numThreads);
	threadpool->start();

	std::thread producerThread([&]() {
		for (int i = 0; i < 5; i++)
		{
			ThreadUtils::Runnable<int64_t> *runnable = new ThreadUtils::Runnable<int64_t>(FirstStage, i);
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			threadpool->feedQueue(runnable);
		}
	});

	std::thread consumerThread([&]() {
		for (int i = 0; i < 5; i++)
		{
			int64_t val = threadpool->fetchFromBuffer();
			std::cout << "Finished Processing: " << val << std::endl;
		}
	});

	producerThread.join();
	consumerThread.join();
	threadpool->stop();
	return 0;
}