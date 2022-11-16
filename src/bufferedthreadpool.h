/*
 * Copyright (C) Evan Stoddard.
 */

/**
 * @file bufferedthreadpool.h
 * @author Evan Stoddard
 * @brief Threadpool with an input and output buffer
 */

#ifndef BUFFEREDTHREADPOOL_H_
#define BUFFEREDTHREADPOOL_H_

#include "threadpool.h"

namespace ThreadUtils
{
	template<typename T>
	class BufferedThreadpool: public Threadpool
	{
	public:

		/**
		 * @brief Construct a new Buffered Threadpool object
		 *
		 * @param numThreads Number of worker threads to spin up
		 */
		explicit BufferedThreadpool(uint32_t numThreads) :
			_activeProcesses(0),
			Threadpool(numThreads)
		{

		}

		/**
		 * @brief Feed input worker queue
		 *
		 */
		void feedQueue(AbstractRunnable *runnable)
		{
			// Take input mutex
			std::unique_lock<std::mutex> l(_queueMutex);

			// Push runnable
			_inputQueue.emplace_back(runnable);

			// Release mutex and signal
			l.unlock();
			_inputCV.notify_all();
		}

		/**
		 * @brief Blocking call to fetch output of type T from output buffer
		 *
		 * @return T Return from back of buffer
		 */
		T fetchFromBuffer()
		{
			// Take lock
			std::unique_lock<std::mutex> l(_outputMutex);

			// Wait on condition variable
			_outputSignal.wait(l, [&]() {
				return !_poolRunning || !_outputBuffer.empty();
			});

			// If pool killed return empty type
			if (!_poolRunning)
			{
				return T();
			}

			// Get output from buffer
			T out = _outputBuffer.front();
			_outputBuffer.pop_front();

			// Release lock
			l.unlock();

			// Return output
			return out;
		}

		/**
		 * @brief Feed output buffer
		 *
		 * @param value Value to feed buffer
		 */
		void feedOutputBuffer(T value)
		{
			// Take lock
			std::unique_lock<std::mutex> l(_outputMutex);

			// Push to buffer and decrement active processing counters
			_outputBuffer.emplace_back(value);
			_activeProcesses--;

			// Release lock
			l.unlock();

			// Notify output buffer
			_outputSignal.notify_one();
		}

	protected:
		/**
		 * @brief Predicate for determining if processing thread to be run
		 *
		 */
		virtual bool inputPredicate() override
		{
			return !_poolRunning || !_inputQueue.empty() || !_queue.empty();
		}

		/**
		 * @brief Process run in threads
		 *
		 */
		virtual void threadRunner() override
		{
			// While pool active
			while (poolRunning())
			{
				// Grab lock
				std::unique_lock<std::mutex> l(_queueMutex);

				// Pointer to runnable
				AbstractRunnable *runnable = nullptr;

				// Wait for change in queue or pool status
				_inputCV.wait(l, [&](){ return inputPredicate(); });

				// If pool killed
				if (!poolRunning())
				{
					// Release lock
					l.unlock();
					break;
				}

				// If threadpool has capacity to pull from input queue
				if (!_inputQueue.empty() && _activeProcesses != _numThreads)
				{
					runnable = _inputQueue.front();
					_inputQueue.pop_front();
					_activeProcesses++;
				}
				else if (!_queue.empty())
				{
					runnable = _queue.front();
					_queue.pop_front();
				}
				else
				{
					l.unlock();
					continue;
				}

				// Release lock
				l.unlock();

				// Execute runnable
				runnable->run();

				// Delete runnable
				delete runnable;
			}
		}

	protected:
		/// @brief Mutex to lock output queue
		std::mutex _outputMutex;

		/// @brief Condition variable for data added to back buffer
		std::condition_variable _outputSignal;

		/// @brief Input Runnable Queue
		std::deque<AbstractRunnable*> _inputQueue;

		/// @brief Output buffer
		std::deque<T> _outputBuffer;

		/// @brief Active processes
		std::atomic_uint32_t _activeProcesses;

	};
};

#endif /* BUFFEREDTHREADPOOL_H_ */