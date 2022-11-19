/*
 * Copyright (C) Evan Stoddard.
 */

/**
 * @file runnable.h
 * @author Evan Stoddard
 * @brief Runnable object
 */

#ifndef RUNNABLE_H_
#define RUNNABLE_H_

#include <functional>
#include <tuple>
#include <utility>

namespace ThreadUtils
{
	/**
	 * @brief Abstract representatation of runnable
	 *
	 */
	class AbstractRunnable
	{
	public:
		virtual ~AbstractRunnable() {};
		virtual void run() = 0;
	};

	/**
	 * @brief Runnable object to handle execution of functor with ...params
	 *
	 * @tparam ReturnType Return type of functor
	 * @tparam Params Type of arguments for functor
	 */
	template <typename ...Params>
	class Runnable : public AbstractRunnable
	{
	public:
		/**
		 * @brief Construct a new Runnable object
		 *
		 * @param f Functor to run
		 * @param params Parameters to pass to functor
		 */
		explicit Runnable(const std::function<void(Params...)> &&f, Params... params) :
			_function(std::move(f)),
			_params(params...) {}

		/**
		 * @brief Destroy the Runnable object
		 *
		 */
		virtual ~Runnable() {}

		/**
		 * @brief Execute task
		 *
		 */
		void run() override
		{
			// Execute function defined by functor with parameters
			callWithParams(_function, _params);
		}

	private:
		template<typename F, typename Tuple, size_t ...S >
		void appleTupleImpl(F&& fn, Tuple&& t, std::index_sequence<S...>)
		{
			std::forward<F>(fn)(std::get<S>(std::forward<Tuple>(t))...);
		}

		template<typename F, typename Tuple>
		void callWithParams(F&& fn, Tuple&& t)
		{
			std::size_t constexpr tSize = std::tuple_size<typename std::remove_reference<Tuple>::type>::value;
			appleTupleImpl
			(
				std::forward<F>(fn),
				std::forward<Tuple>(t),
				std::make_index_sequence<tSize>()
			);
		}

	private:
		std::function<void(Params...)> _function;
		std::tuple<Params...> _params;
	};

};

#endif /* RUNNABLE_H_ */