/*
 * Copyright (C) Evan Stoddard.
 */

/**
 * @file orderedbufferedthreadpool.hpp
 * @author Evan Stoddard
 * @brief Threadpool with input and output queue.  Output ordered in same order as input.
 */

#ifndef ORDEREDBUFFEREDTHREADPOOL_HPP_
#define ORDEREDBUFFEREDTHREADPOOL_HPP_

#include "bufferedthreadpool.hpp"

namespace ThreadUtils
{
	template <typename T, typename TagType>
	class OrderedBufferedThreadpool : public BufferedThreadpool<T>
	{
	public:
		/**
		 * @brief Construct a new Ordered Buffered Threadpool object
		 *
		 * @param numThreads Number of threads
		 */
		explicit OrderedBufferedThreadpool(uint32_t numThreads) :
			BufferedThreadpool<T>(numThreads),
			_outputContainers(numThreads),
			_maxInputQueueSize(-1)
		{
		}

		/**
		 * @brief Feed input queue with runnable and tag
		 *
		 * @param runnable Runnable
		 * @param tag Tag
		 */
		void feedQueue(AbstractRunnable *runnable, TagType tag)
		{
			// Take input mutex
			std::unique_lock<std::mutex> l(BufferedThreadpool<T>::_queueMutex);

			if (BufferedThreadpool<T>::_inputQueue.size() >= _maxInputQueueSize)
			{
				l.unlock();
				return;
			}

			// Push runnable and process container
			Container container;
			container.tag = tag;
			BufferedThreadpool<T>::_inputQueue.emplace_back(runnable);
			_inputContainers.emplace_back(container);

			std::unique_lock<std::mutex> ol(BufferedThreadpool<T>::_outputMutex);
			_outputOrder.emplace_back(tag);
			ol.unlock();

			// Release mutex and signal condition variable
			l.unlock();
			BufferedThreadpool<T>::_inputCV.notify_all();
		}

		/**
		 * @brief Feed output queue
		 *
		 * @param value Value to feed queue
		 * @param tag Tag to order output
		 */
		void feedOutputQueue(T value, TagType tag)
		{
			// Update output buffer
			updateOutputBuffer(value, tag, true);
		}

		/**
		 * @brief Invalidate tag
		 *
		 * @param tag Tag to invalidate
		 */
		void invalidateTag(TagType tag)
		{
			updateOutputBuffer(T(), tag, false);
		}

		/**
		 * @brief Set the Max Input Queue Size object
		 *
		 * @param maxSize Max size of queue (-1 unlimited)
		 */
		void setMaxInputQueueSize(uint64_t maxSize)
		{
			_maxInputQueueSize = maxSize;
		}

	protected:
		/**
		 * @brief Process run in threads
		 *
		 */
		virtual void threadRunner() override
		{
			// While pool active
			while (BufferedThreadpool<T>::poolRunning())
			{
				// Grab lock
				std::unique_lock<std::mutex> l(BufferedThreadpool<T>::_queueMutex);

				// Pointer to runnable
				AbstractRunnable *runnable = nullptr;

				// Wait for change in queue or pool status
				BufferedThreadpool<T>::_inputCV.wait(l, [&](){ return BufferedThreadpool<T>::inputPredicate(); });

				// If pool killed
				if (!BufferedThreadpool<T>::poolRunning())
				{
					// Release lock
					l.unlock();
					break;
				}

				// Attempt to take output lock
				std::unique_lock<std::mutex> ol(BufferedThreadpool<T>::_outputMutex);

				// If threadpool has capacity to pull from input queue
				if
				(
					!BufferedThreadpool<T>::_inputQueue.empty() &&
					BufferedThreadpool<T>::_activeProcesses != BufferedThreadpool<T>::_numThreads
				)
				{
					runnable = BufferedThreadpool<T>::_inputQueue.front();
					BufferedThreadpool<T>::_inputQueue.pop_front();
					BufferedThreadpool<T>::_activeProcesses++;

					// Find and update first available container
					for (auto &container : _outputContainers)
					{
						if (container.slotAvailable)
						{
							container = _inputContainers.front();
							container.slotAvailable = false;
							_inputContainers.pop_front();
							break;
						}
					}

				}
				else if (!BufferedThreadpool<T>::_queue.empty())
				{
					runnable = BufferedThreadpool<T>::_queue.front();
					BufferedThreadpool<T>::_queue.pop_front();
				}
				else
				{
					l.unlock();
					ol.unlock();
					continue;
				}

				// Release output lock
				ol.unlock();

				// Release lock
				l.unlock();

				// Execute runnable
				runnable->run();

				// Delete runnable
				delete runnable;
			}
		}

	private:
		/**
		 * @brief Update the output buffer
		 *
		 * @param value Value to feed buffer
		 * @param tag Tag associated with update
		 * @param valid Whether value valid or not
		 */
		void updateOutputBuffer(T value, TagType tag, bool valid)
		{
			// Take lock
			std::unique_lock<std::mutex> l(BufferedThreadpool<T>::_outputMutex);

			// Bool indicating container found
			bool foundContainer = false;
			Container matchingContainer;

			// Iterate through output containers
			for (auto &container : _outputContainers)
			{
				if (container.tag == tag)
				{
					// Update container values
					container.value = value;
					container.finishedProcessing = true;
					container.valid = valid;
					container.slotAvailable = (container.tag == _outputOrder.front());

					// Indicate container found
					matchingContainer = container;
					foundContainer = true;
				}
			}

			// Throw exception if tag not found
			if (!foundContainer)
			{
				l.unlock();
				throw std::invalid_argument("Tag does not exist.");
			}

			// If slot ready to be available again
			if (matchingContainer.slotAvailable)
			{
				// Add value to output queue if valid
				if (valid)
				{
					BufferedThreadpool<T>::_outputBuffer.emplace_back(matchingContainer.value);
				}

				// Decrement active process counter and pop output order queue
				BufferedThreadpool<T>::_activeProcesses--;
				_outputOrder.pop_front();

				// Check remaining values in output containers
				while(!_outputOrder.empty())
				{
					bool found = false;

					// Iterate through containers
					for (auto &container : _outputContainers)
					{
						if (
							container.tag == _outputOrder.front() &&
							container.finishedProcessing &&
							!container.slotAvailable
						)
						{
							// Add value to output queue if available
							if (container.valid)
							{
								BufferedThreadpool<T>::_outputBuffer.emplace_back(container.value);
							}

							// Make slot available
							container.slotAvailable = true;

							// Decrement active process counter and pop output order queue
							BufferedThreadpool<T>::_activeProcesses--;
							_outputOrder.pop_front();
							found = true;
							break;
						}
					}

					if (!found)
					{
						break;
					}
				}
			}

			// Release lock and notify
			l.unlock();
			BufferedThreadpool<T>::_outputSignal.notify_all();
		}

	private:
		/**
		 * @brief Container of output to hold tag, status, and value
		 *
		 */
		struct Container
		{
			Container() :
				value(),
				tag(),
				finishedProcessing(false),
				valid(false),
				slotAvailable(true)
			{}

			T value;
			TagType tag;
			bool finishedProcessing;
			bool valid;
			bool slotAvailable;
		};

		/// @brief Input queue for process containers
		std::deque<Container> _inputContainers;

		/// @brief Vector containing output process containers
		std::vector<Container> _outputContainers;

		/// @brief Order of output
		std::deque<TagType> _outputOrder;

		uint64_t _maxInputQueueSize;

	};
};

#endif /* ORDEREDBUFFEREDTHREADPOOL_HPP_ */
