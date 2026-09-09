#pragma once

#include "DllHelper.hpp"

#include "astra/json.hpp"
#include "astra/traits.hpp"

#include <functional>
#include <mutex>
#include <optional>
#include <type_traits>

/**
 * @brief The namespace containing all of the reactive state machinery for the UI system
 */
namespace Cacao::Reactivity {
	/**
	 * @brief A small base class for all state-related dependency objects
	 */
	class CACAO_API Dependency {
	  protected:
		void EvalReadOk();
		void EvalWriteOk();
	};

	/**
	 * @brief A small base class for all objects that may be declared as a side effect output
	 */
	class CACAO_API EffectOutput {};

	/**
	 * @brief A source of reactive state
	 */
	template<astra::Serializable T>
	class CACAO_API State : public Dependency, public EffectOutput {
	  public:
		/**
		 * @brief Create some state with no initial value
		 */
		State()
			requires std::is_default_constructible_v<T>
		{}

		/**
		 * @brief Create some state with an initial value
		 *
		 * @param initial The initial value to store
		 */
		template<typename... Args>
		State(Args&&... args)
			requires std::is_constructible_v<T, Args&&...>
		  : storage(std::forward<Args>(args)...) {}

		/**
		 * @brief Get the current value
		 *
		 * @return The current held value
		 */
		operator const T&() const {
			EvalReadOk();
			std::lock_guard lk(storageGuard);
			return storage;
		}

		/**
		 * @brief Access the current value in a read-only manner
		 */
		const T* operator->() const {
			EvalReadOk();
			std::lock_guard lk(storageGuard);
			return &storage;
		}

		/**
		 * @brief Wholly replace the currently held value with a copy of an existing object
		 *
		 * @param value The new value
		 */
		State& operator=(const T& value)
			requires std::is_copy_assignable_v<T>
		{
			EvalWriteOk();
			std::lock_guard lk(storageGuard);
			storage = value;
			++writeCounter;
			//TODO: set global dirty flag
		}

		/**
		 * @brief Wholly replace the currently held value with a new one
		 *
		 * @param value The new value
		 */
		State& operator=(T&& value)
			requires std::is_move_assignable_v<T>
		{
			EvalWriteOk();
			std::lock_guard lk(storageGuard);
			storage = value;
			++writeCounter;
			//TODO: set global dirty flag
		}

		/**
		 * @brief A small proxy object for controlling access to state mutation
		 */
		class MutationProxy {
		  public:
			/**
			 * @brief Get the current value
			 *
			 * @return The current held value
			 */
			operator T&() {
				return *ref;
			}

			/**
			 * @brief Access the current value in a writable manner
			 */
			T* operator->() {
				return ref;
			}

			~MutationProxy() {
				lock.unlock();
				++(*writeCounter);
				//TODO: set global dirty flag
			}

		  private:
			MutationProxy() {}
			T* ref;
			unsigned int* writeCounter;
			std::unique_lock<std::mutex> lock;
			friend class State;
		};

		/**
		 * @brief Modify the contents of the currently held value
		 */
		MutationProxy Modify() {
			EvalWriteOk();
			MutationProxy proxy;
			proxy.lock = std::unique_lock<std::mutex>(storageGuard);
			proxy.ref = &storage;
			proxy.writeCounter = &writeCounter;
			return proxy;
		}

		///@cond
		State(const State& other)
			requires std::is_copy_constructible_v<T>
		  : storage((std::lock_guard {other.storageGuard}, other.storage)) {}
		State& operator=(const State& other)
			requires std::is_copy_assignable_v<T>
		{
			other.EvalReadOk();
			EvalWriteOk();
			if(this != &other) {
				std::lock_guard lk(storageGuard);
				std::lock_guard lk2(other.storageGuard);
				storage = other;
				++writeCounter;
				//TODO: set global dirty flag
			}
			return *this;
		}
		State(State&& other)
			requires std::is_move_constructible_v<T>
		  : storage((std::lock_guard {other.storageGuard}, std::move(other.storage))) {}
		State& operator=(State&& other)
			requires std::is_move_assignable_v<T>
		{
			other.EvalWriteOk();
			EvalWriteOk();
			if(this != &other) {
				std::lock_guard lk(storageGuard);
				std::lock_guard lk2(other.storageGuard);
				storage = std::move(other);
				++writeCounter;
				//TODO: set global dirty flag
			}
			return *this;
		}
		///@endcond

	  private:
		T storage;
		std::mutex storageGuard;
		unsigned int writeCounter = 0;
		//TODO: friend class WhateverChecksTheWriteCounter;
	};

	/**
	 * @brief A piece of state whose value is computed when needed and cached between updates
	 */
	template<astra::Serializable T>
	class CACAO_API Computed : public Dependency {
	  public:
		/**
		 * @brief Create a new piece of computed state from a source
		 *
		 * @param inputs The dependencies that should trigger a re-evaluation of the value; only these values can be accessed during computation
		 * @param evaluator The function to perform the evaluation
		 */
		Computed(std::initializer_list<Dependency*> inputs, std::function<T(void)> evaluator)
		  : evaluator(evaluator) {
			//TODO: register self as a dependency and do cycle checks
		}

		/**
		 * @brief Get the current value
		 *
		 * @return The current held value
		 */
		operator const T&() const {
			EvalReadOk();
			return result.value();
		}

		/**
		 * @brief Access the current value in a read-only manner
		 */
		const T* operator->() const {
			EvalReadOk();
			return &result.value();
		}

	  private:
		std::function<T(void)> evaluator;
		std::optional<T> result;
		unsigned int updateCounter;
		//TODO: friend class WhateverTriggersEvaluate;

		void Evaluate() {
			++updateCounter;
			result = evaluator();
		}
	};

	/**
	 * @brief A side effect callback triggered by a change in state
	 */
	class CACAO_API SideEffect {
	  public:
		/**
		 * @brief Create a new piece of computed state from a source
		 *
		 * @param inputs The dependencies that should trigger a re-evaluation of the value; only these values and outputs can be accessed during execution
		 * @param outputs The state and callbacks that may be modified by this effect; only these values and inputs can be accessed during execution
		 * @param callback The function to run when the state changes
		 */
		SideEffect(std::initializer_list<Dependency*> inputs, std::initializer_list<EffectOutput*> outputs, std::function<void(void)> callback)
		  : callback(callback) {
			//TODO: register self as a dependency and do cycle checks
		}

	  private:
		std::function<void(void)> callback;
		//TODO: friend class WhateverCallsTheCallback;
	};

	/**
	 * @brief A bridge to access data originating outside of the reactivity system
	 */
	template<astra::Serializable T>
		requires std::is_move_constructible_v<T>
	class CACAO_API ExternalBinding : public Dependency {
	  public:
		/**
		 * @brief Create a new external data binding
		 *
		 * @param fetcher The function that retrieves the current state of the external data
		 * @param fluid Whether the value should be considered to have always changed
		 *
		 * @warning Using an external binding with @c fluid enabled will force the UI to redraw on <b>every tick</b> because there will always be dirty state. Use with caution!
		 */
		ExternalBinding(std::function<T(void)> fetcher, bool fluid = false)
		  : fetcher(fetcher), fluid(fluid) {}

		/**
		 * @brief Get the current value
		 *
		 * @return The current held value
		 */
		operator const T&() const {
			EvalReadOk();
			return *cache;
		}

		/**
		 * @brief Access the current value in a read-only manner
		 */
		const T* operator->() const {
			EvalReadOk();
			return cache.get();
		}

	  private:
		std::function<T()> fetcher;
		std::unique_ptr<T> cache;
		std::size_t cacheHash = 0;
		bool fluid;
		//TODO: friend class WhateverCallsCheckRefresh

		void CheckRefresh() {
			T temp = fetcher();
			if(!fluid) {
				astra::SerializedSubstitute<T> sub(temp);
				std::size_t hash = std::hash<std::string> {}(astra::json::toString(&sub));
				if(hash == cacheHash) return;
				cacheHash = hash;
			}
			cache = std::make_unique<T>(std::move(temp));
			//TODO: set global dirty flag
		}
	};
}