/*
 * Copyright (C) Evan Stoddard.
 */

/**
 * @file threadpool.cpp
 * @author Evan Stoddard
 * @brief Threadpool class
 */

#include "threadpool.h"

// Use ThreadUtils namespace
using namespace ThreadUtils;

/**
 * @brief Construct a new Threadpool:: Threadpool object
 *
 * @param numThreads Number of threads in thread ool
 */
Threadpool::Threadpool(uint32_t numThreads) :
	_numThreads(numThreads),
	_poolRunning(false)
{
}

/**
 * @brief Enqueue runnable onto runnable queue
 *
 * @param runnable Runnable object
 */
void Threadpool::enqueue(AbstractRunnable *runnable)
{
	// Take lock
	std::unique_lock<std::mutex> lock(_queueMutex);

	// Push to queue
	_queue.emplace_back(runnable);

	// Release lock
	lock.unlock();

	// Notify threads of new data
	_inputCV.notify_all();
}

/**
 * @brief Starts threadpool
 *
 */
void Threadpool::start()
{
	// Don't do anything if threads already exist
	if (!_threads.empty())
	{
		return;
	}

	// Set running flag
	_poolRunning = true;

	// Create threads
	for (uint32_t i = 0; i < _numThreads; i++)
	{
		std::thread *thread = new std::thread(&Threadpool::threadRunner, this);
		_threads.push_back(thread);
	}
}

/**
 * @brief Stops threadpool
 *
 */
void Threadpool::stop()
{
	// Check if thread vector empty
	if (_threads.empty())
	{
		return;
	}

	// Set flag to false and notify threads
	_poolRunning = false;
	_inputCV.notify_all();

	// Wait for thread to finish and delete
	for (auto thread : _threads)
	{
		thread->join();
		delete thread;
	}

	// Clear array of threads
	_threads.clear();
}

/**
 * @brief Predicate dicating whether thread runner should perform action
 *
 * @return Condition that requires thread intervention
 */
bool Threadpool::inputPredicate()
{
	return !_queue.empty() || !poolRunning();
}

/**
 * @brief Function run in threads
 *
 */
void Threadpool::threadRunner()
{
	// While pool active
	while (poolRunning())
	{
		// Grab lock
		std::unique_lock<std::mutex> l(_queueMutex);

		// Pointer to runnable
		AbstractRunnable *runnable = nullptr;

		if (!poolRunning())
		{
			break;
			l.unlock();
		}

		// Wait for change in queue or pool status
		_inputCV.wait(l, [&](){ return inputPredicate(); });

		// If pool killed
		if (!poolRunning())
		{
			// Release lock
			l.unlock();
			break;
		}

		if (_queue.empty())
		{
			l.unlock();
			continue;
		}

		runnable = _queue.front();
		_queue.pop_front();
		l.unlock();

		// Execute runnable
		runnable->run();

		// Delete runnable when done
		delete runnable;
	}
}

/**
 * @brief Destroy the Threadpool:: Threadpool object
 *
 */
Threadpool::~Threadpool()
{
	// Stop and delete threads
	stop();
}