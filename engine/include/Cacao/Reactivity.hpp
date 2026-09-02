#pragma once

#include "DllHelper.hpp"

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
	class CACAO_API Dependency {};

	/**
	 * @brief A source of reactive state
	 */
	template<astra::Reflectable T>
	class CACAO_API State : public Dependency {
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
			std::lock_guard lk(storageGuard);
			return storage;
		}

		/**
		 * @brief Access the current value in a read-only manner
		 */
		const T* operator->() const {
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
			std::lock_guard lk(storageGuard);
			storage = value;
		}

		/**
		 * @brief Wholly replace the currently held value with a new one
		 *
		 * @param value The new value
		 */
		State& operator=(T&& value)
			requires std::is_move_assignable_v<T>
		{
			std::lock_guard lk(storageGuard);
			storage = value;
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
				//TODO: set dirty flag
			}

		  private:
			MutationProxy() {}
			T* ref;
			std::unique_lock<std::mutex> lock;
			friend class State;
		};

		/**
		 * @brief Modify the contents of the currently held value
		 */
		MutationProxy Modify() {
			MutationProxy proxy;
			proxy.lock = std::unique_lock<std::mutex>(storageGuard);
			proxy.ref = &storage;
			return proxy;
		}

		///@cond
		State(const State& other)
			requires std::is_copy_constructible_v<T>
		  : storage((std::lock_guard {other.storageGuard}, other.storage)) {}
		State& operator=(const State& other)
			requires std::is_copy_assignable_v<T>
		{
			if(this != &other) {
				std::lock_guard lk(storageGuard);
				std::lock_guard lk2(other.storageGuard);
				storage = other;
			}
			return *this;
		}
		State(State&& other)
			requires std::is_move_constructible_v<T>
		  : storage((std::lock_guard {other.storageGuard}, std::move(other.storage))) {}
		State& operator=(State&& other)
			requires std::is_move_assignable_v<T>
		{
			if(this != &other) {
				std::lock_guard lk(storageGuard);
				std::lock_guard lk2(other.storageGuard);
				storage = std::move(other);
			}
			return *this;
		}
		///@endcond

	  private:
		T storage;
		std::mutex storageGuard;
	};

	/**
	 * @brief A piece of state whose value is computed when needed and cached between updates
	 */
	template<astra::Reflectable T>
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
			return result.value();
		}

		/**
		 * @brief Access the current value in a read-only manner
		 */
		const T* operator->() const {
			return &result.value();
		}

	  private:
		std::function<T(void)> evaluator;
		std::optional<T> result;
		unsigned int updateCounter;
		//TODO: friend class WhateverTriggersEvaluate

		void Evaluate() {
			++updateCounter;
			result = evaluator();
		}
	};
}