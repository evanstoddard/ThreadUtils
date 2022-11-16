/*
 * Copyright (C) Evan Stoddard.
 */

/**
 * @file threadpool.h
 * @author Evan Stoddard
 * @brief Threadpool class
 */

#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <stdint.h>
#include <vector>
#include <deque>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include "runnable.h"

namespace ThreadUtils
{

	class Threadpool
	{
	public:
		explicit Threadpool(uint32_t numThreads);
		~Threadpool();
		void push(AbstractRunnable *runnable);
		void start();
		void stop();
		bool poolRunning() { return _poolRunning; };

	protected:
		virtual void threadRunner();
		virtual bool inputPredicate();

	protected:
		/// @brief Size of thread pool
		uint32_t _numThreads;

		/// @brief Thread pool currently active
		std::atomic_bool _poolRunning;

		/// @brief Queue of runnables
		std::deque<AbstractRunnable*> _queue;

		/// @brief Vector of threads
		std::vector<std::thread*> _threads;

		/// @brief Mutex synchronizing access to runnable queue
		std::mutex _queueMutex;

		/// @brief Condition variable to notify threads
		std::condition_variable _inputCV;
	};

};

#endif /* THREADPOOL_H_ */