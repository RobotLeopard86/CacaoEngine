#pragma once

#include <exception>
#include <string>
#include <vector>
#include <sstream>
#include <map>

#include "Core/Log.hpp"

//Quick utility to check for a condition and throw an exception if false
//Write first argument as a normal condition, it will be wrapped in parentheses
#define CheckException(condition, exceptionCode, exceptionDescription) if(!(condition)) {\
	Exception e{exceptionDescription, exceptionCode};\
	Logging::EngineLog(e.what(), LogLevel::Error);\
	throw e;\
	}

namespace Cacao {
	//A Cacao Engine exception
	class Exception : public std::exception {
	public:
		Exception(std::string description, unsigned int exceptionCode)
			: error(description), code(exceptionCode) {
			std::stringstream s;
			s << "Cacao Exception - \"" << error << "\" (Code " << code << ": " << GetCodeMeaning() << ")";
			detailedMsg = s.str();
		}
		
		//Get a detailed exception string, inherited from std::exception
		const char* what() const noexcept override {
			return detailedMsg.c_str();
		}

		//Return the error code as an unsigned int for use in switch statements
		operator unsigned int() {
			return code;
		}

		//Utility methods
		const std::string& GetRawError() { return error; }
		unsigned int GetRawCode() { return code; }
		const std::string& GetCodeMeaning() { return exceptionCodeMap[code]; }

		//Register an exception code
		//Returns a boolean of whether the operation succeeded
		//Will fail if either the code has already been registered or the definition is an empty string
		static bool RegisterExceptionCode(unsigned int code, std::string definition) {
			if(exceptionCodeMap.contains(code)) return false;
			if(definition == "") return false;
			exceptionCodeMap.insert_or_assign(code, definition);
			return true;
		}

		//Get the meaning of an exception code
		//Returns an empty string if no code found
		static std::string GetExceptionCodeMeaning(unsigned int code){
			if(!exceptionCodeMap.contains(code)) return "";
			return exceptionCodeMap[code];
		}

		//Get the code of an exception from meaning
		//Returns maximum integer value if no meaning found
		static unsigned int GetExceptionCodeFromMeaning(std::string meaning){
			auto ecmEntry = std::find_if(exceptionCodeMap.cbegin(), exceptionCodeMap.cend(), [meaning](const std::pair<unsigned int, std::string>& kv){
				return kv.second.compare(meaning) == 0;
			});
			return (ecmEntry != exceptionCodeMap.cend() ? ecmEntry->first : INT_MAX);
		}

		//Unregister an exception code (shouldn't be used commonly)
		//Returns a boolean of whether the operation succeeded
		//Will fail if the supplied code has not been registered
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