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
		virtual void run() = 0;
	};

	template <typename T>
	class Runnable{};

	template <typename ReturnType, typename... Params>
	class Runnable<ReturnType(Params...)> : public AbstractRunnable
	{
	public:
		/**
		 * @brief Construct a new Runnable object
		 *
		 * @param f Functor to run
		 * @param params Parameters to pass to functor
		 */
		explicit Runnable(std::function<void(Params...)> f, Params... params) :
			_function(std::move(f)),
			_params(params...) {}

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
		ReturnType appleTupleImpl(F&& fn, Tuple&& t, std::index_sequence<S...>)
		{
			return std::forward<F>(fn)(std::get<S>(std::forward<Tuple>(t))...);
		}

		template<typename F, typename Tuple>
		ReturnType callWithParams(F&& fn, Tuple&& t)
		{
			std::size_t constexpr tSize = std::tuple_size<typename std::remove_reference<Tuple>::type>::value;
			return appleTupleImpl
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