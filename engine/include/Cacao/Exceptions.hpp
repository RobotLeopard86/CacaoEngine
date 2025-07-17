#pragma once

#include "DllHelper.hpp"
#include "Log.hpp"

#include <string>
#include <sstream>

namespace Cacao {
	///@cond

	class CACAO_API Exception : public std::exception {
	  public:
		const char* what() const noexcept final override {
			return msg.c_str();
		}

	  protected:
		std::string msg;

		Exception() : msg("") {}
		virtual ~Exception() {}
	};

#define DEF_EXCEPTION(cls, tp)                                   \
	class CACAO_API cls##Exception : public Exception {          \
	  public:                                                    \
		cls##Exception(std::string m) : Exception() {            \
			msg = makeMsg(m);                                    \
		}                                                        \
		~cls##Exception() {}                                     \
		std::string makeMsg(const std::string& m) {              \
			std::stringstream ss;                                \
			ss << "(Cacao Engine " << tp << " Exception) " << m; \
			return ss.str();                                     \
		}                                                        \
	};

	DEF_EXCEPTION(External, "External")
	DEF_EXCEPTION(FileNotFound, "File Not Found")
	DEF_EXCEPTION(IO, "I/O")
	DEF_EXCEPTION(FileOpen, "File Open")
	DEF_EXCEPTION(InvalidYAML, "Invalid YAML")
	DEF_EXCEPTION(BadInitState, "Bad Initialization State")
	DEF_EXCEPTION(BadRealizeState, "Bad Realization State")
	DEF_EXCEPTION(BadState, "Bad State")
	DEF_EXCEPTION(BadThread, "Bad Thread")
	DEF_EXCEPTION(BadType, "Bad Type")
	DEF_EXCEPTION(BadValue, "Bad Value")
	DEF_EXCEPTION(NonexistentValue, "Nonexistent Value")
	DEF_EXCEPTION(ExistingValue, "Existing Value")
	DEF_EXCEPTION(Misc, "Miscellaneous")

#undef DEF_EXCEPTION

	///@endcond

	/**
	 * @brief Checks if the condition is true, and throws an exception if not
	 *
	 * @param condition The condition to check.
	 * @param message The human-readable message to print if the condition is false
	 *
	 */
	template<typename E>
		requires std::is_base_of_v<Exception, E>
	CACAO_API void Check(bool condition, std::string message) {
		if(!condition) {
			E ex(message);
			Logger::Engine(Logger::Level::Error) << ex.what();
			throw ex;
		}
	}

	/**
	 * @brief Checks if the shared_ptr contains a value, and throws an exception if not
	 *
	 * @param ptr The shared_ptr to check for a value.
	 * @param message The human-readable message to print if the condition is false
	 *
	 */
	template<typename P, typename E>
		requires std::is_base_of_v<Exception, E>
	CACAO_API void Check(std::shared_ptr<P> ptr, std::string message) {
		Check<E>((bool)ptr, message);
	}
}