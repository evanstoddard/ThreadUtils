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
#include "runnable.hpp"
#include <iostream>

namespace ThreadUtils
{

	class Threadpool
	{
	public:
		/**
		 * @brief Construct a new Threadpool:: Threadpool object
		 *
		 * @param numThreads Number of threads in thread ool
		 */
		explicit Threadpool(uint32_t numThreads) :
			_numThreads(numThreads),
			_poolRunning(false)
		{
		}

		/**
		 * @brief Destroy the Threadpool object
		 *
		 */
		~Threadpool()
		{
			// Stop and delete threads
			stop();
		}

		/**
		 * @brief Enqueue runnable onto runnable queue
		 *
		 * @param runnable Runnable object
		 */
		void enqueue(AbstractRunnable *runnable)
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
		 * @brief Create and enqueue Runnable
		 *
		 * @tparam _Callable Function to run
		 * @tparam Params Parameter types
		 * @param func Function to run
		 * @param params Parameters to pass to function
		 */
		template <typename Func, typename ...Params>
		void enqueue_new(Func &&func, Params ...params)
		{
			enqueue(new Runnable<Params...>(std::move(func), params...));
		}

		/**
		 * @brief Starts threadpool
		 *
		 */
		void start()
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
		void stop()
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
		 * @brief Returns whether threadpool is running
		 *
		 * @return true Threadpool running
		 * @return false Threadpool not running
		 */
		bool poolRunning() { return _poolRunning; }

	protected:
		/**
		 * @brief Function run in threads
		 *
		 */
		virtual void threadRunner()
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
		 * @brief Predicate dicating whether thread runner should perform action
		 *
		 * @return Condition that requires thread intervention
		 */
		virtual bool inputPredicate()
		{
			return !_queue.empty() || !poolRunning();
		}

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