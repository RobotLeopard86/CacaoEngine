#pragma once

#include <exception>
#include <string>
#include <vector>
#include <sstream>
#include <map>

#include "Core/Log.hpp"

//Quick utility to check for a condition and throw an exception if false
//Write first argument as a normal condition, it will be wrapped in parentheses
/**
 * @brief Check if the condition is true, and throw an exception if not
 *
 * @param condition The condition to check. No parenthetical wrapping necessary.
 * @param exceptionCode The exception code to use. Must have been registered prior to use.
 * See the page "Exception Codes" in the manual for a list of codes.
 * @param exceptionDescription The human-readable message to print if the condition is false
 *
 * @throws Cacao::Exception Thrown if the condition is false
 */
#define CheckException(condition, exceptionCode, exceptionDescription) \
	if(!(condition)) {                                                 \
		Cacao::Exception ex {exceptionDescription, exceptionCode};     \
		Cacao::Logging::EngineLog(ex.what(), LogLevel::Error);         \
		throw ex;                                                      \
	}

namespace Cacao {
	/**
	 * @brief A Cacao Engine exception. Do not subclass this. Prefer over standard library exceptions.
	 *
	 * @note Recommended to use @ref CheckException to generate exceptions.
	 */
	class Exception : public std::exception {
	  public:
		/**
		 * @brief Create a new exception
		 *
		 * @param exceptionCode The exception code to use. Must have been registered prior to use. See the page "Exception Codes" in the manual for more details.
		 * @param exceptionDescription The human-readable message to print if the condition is false
		 */
		Exception(std::string description, unsigned int exceptionCode)
		  : error(description), code(exceptionCode) {
			std::stringstream s;
			s << "Cacao Exception - \"" << error << "\" (Code " << code << ": " << GetCodeMeaning() << ")";
			detailedMsg = s.str();
		}

		/**
		 * @brief Get the exception message
		 * @details This method is inherited from std::exception.
		 *
		 * @note Similar to @ref GetRawError, but returns a C string
		 *
		 * @return The exception message as a C string
		 */
		const char* what() const noexcept override {
			return detailedMsg.c_str();
		}

		///@brief Return the error code as an unsigned int for use in switch statements
		operator unsigned int() {
			return code;
		}

		/**
		 * @brief Get the exception message
		 *
		 * @note Similar to @ref what, but returns a std::string
		 *
		 * @return The exception message as a std::string
		 */
		const std::string& GetRawError() {
			return error;
		}

		/**
		 * @brief Get the exception code
		 *
		 * @return The exception code
		 */
		unsigned int GetRawCode() {
			return code;
		}

		/**
		 * @brief Get the meaning of the code
		 *
		 * @return The meaning of the exception code
		 */
		const std::string& GetCodeMeaning() {
			return exceptionCodeMap[code];
		}

		/**
		 * @brief Register a new exception code
		 *
		 * @param code The exception code to associate with the meaning
		 * @param definition The meaning of the exception code
		 *
		 * @return If the code could be added. Fails if definition is an empty string or if code has already been registered
		 */
		static bool RegisterExceptionCode(unsigned int code, std::string definition) {
			if(exceptionCodeMap.contains(code)) return false;
			if(definition == "") return false;
			exceptionCodeMap.insert_or_assign(code, definition);
			return true;
		}

		/**
		 * @brief Get the meaning of an exception code
		 *
		 * @param code The exception code
		 *
		 * @return The meaning of the code, or an empty string if the code hasn't been registered
		 */
		static std::string GetExceptionCodeMeaning(unsigned int code) {
			if(!exceptionCodeMap.contains(code)) return "";
			return exceptionCodeMap[code];
		}

		/**
		 * @brief Get an exception code from its meaning
		 *
		 * @param code The meaning of a code
		 *
		 * @return The exception code associated with meaning or UINT_MAX if no matching code was found
		 */
		static unsigned int GetExceptionCodeFromMeaning(std::string meaning) {
			auto ecmEntry = std::find_if(exceptionCodeMap.cbegin(), exceptionCodeMap.cend(), [meaning](const std::pair<unsigned int, std::string>& kv) {
				return kv.second.compare(meaning) == 0;
			});
			return (ecmEntry != exceptionCodeMap.cend() ? ecmEntry->first : UINT_MAX);
		}

		/**
		 * @brief Unregister an exception code
		 *
		 * @param code The exception code to unregister
		 *
		 * @return If the code could be removed. Fails if code has not been registered
		 *
		 * @note Rarely needed
		 */
		static bool UnregisterExceptionCode(unsigned int code) {
			if(!exceptionCodeMap.contains(code)) return false;
			exceptionCodeMap.erase(code);
			return true;
		}

	  private:
		std::string error;
		unsigned int code;

		std::string detailedMsg;

		static std::map<unsigned int, std::string> exceptionCodeMap;
	};

	//Static member definition
	inline std::map<unsigned int, std::string> Exception::exceptionCodeMap = std::map<unsigned int, std::string>();
}
