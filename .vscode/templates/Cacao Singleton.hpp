#pragma once

#include "DllHelper.hpp"

namespace Cacao {
	/**
	 * @brief SingletonDescription
	 */
	class CACAO_API Class {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static Class& Get();

		///@cond
		Class(const Class&) = delete;
		Class(Class&&) = delete;
		Class& operator=(const Class&) = delete;
		Class& operator=(Class&&) = delete;
		///@endcond
	  private:
		Class();
		~Class();
	};
}