#pragma once

#include "Core/DllHelper.hpp"

namespace Cacao {
	/**
	 * @brief Utility that caches changes to an object and applies them on command
	 */
	template<typename T>
	class CACAO_API Flushable {
	  public:
		/**
		 * @brief Create a new flushable
		 *
		 * @param obj The original object
		 */
		explicit Flushable(T& obj)
		  : originalObject(obj), mod(obj) {}

		/**
		 * @brief Access the object copy where changes can be made
		 */
		T* operator->() {
			return &mod;
		}

		/**
		 * @brief Overwrite the object copy with new data
		 */
		void operator=(T newDat) {
			mod = newDat;
		}

		/**
		 * @brief Write the changes back to the original object
		 */
		void Flush() {
			originalObject = mod;
		}

	  private:
		T& originalObject;
		T mod;//Local copy to work on
	};
}