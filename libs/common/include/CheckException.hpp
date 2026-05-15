#pragma once

#include <functional>
#include <stdexcept>
#include <string>

#include "DllHelper.hpp"

/**
 * @brief Quick utility to throw an exception on an error condition
 *
 * The exception will be thrown if the condition evaluates to false
 *
 * @param cond The condition to evaluate
 * @param msg The message for the exception thrown
 * @param unwindFn A function to clean up state before exception throwing should the condition be false
 */
CACAO_API inline void CheckException(bool cond, std::string msg, std::function<void()> unwindFn = []() {}) {
	if(!cond) {
		unwindFn();
		throw std::runtime_error(msg);
	}
}