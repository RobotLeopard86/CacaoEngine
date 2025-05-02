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
		const std::string msg;

		virtual std::string type() const {
			return "";
		}

		std::string makeMsg(const std::string& m) {
			std::stringstream ss;
			ss << "(Cacao Engine " << type() << " Exception) " << msg;
			return ss.str();
		}

		Exception(const std::string& m) : msg(makeMsg(m)) {}
		virtual ~Exception();
	};

#define DEF_EXCEPTION(cls, tp)                          \
	class CACAO_API cls##Exception : public Exception { \
	  public:                                           \
		cls##Exception(std::string m) : Exception(m) {} \
		~cls##Exception() {}                            \
                                                        \
	  protected:                                        \
		std::string type() const override {             \
			return tp;                                  \
		}                                               \
	};

	DEF_EXCEPTION(External, "External")
	DEF_EXCEPTION(FileNotFound, "File Not Found")
	DEF_EXCEPTION(IO, "I/O")
	DEF_EXCEPTION(FileOpen, "File Open")
	DEF_EXCEPTION(InvalidYAML, "Invalid YAML")
	DEF_EXCEPTION(BadInitState, "Bad Initialization State")
	DEF_EXCEPTION(BadGPUState, "Bad GPU State")
	DEF_EXCEPTION(BadState, "Bad State")
	DEF_EXCEPTION(BadThread, "Bad Thread")
	DEF_EXCEPTION(BadType, "Bad Type")
	DEF_EXCEPTION(NonexistentValue, "Nonexistent Value")
	DEF_EXCEPTION(NullValue, "Null Value")
	DEF_EXCEPTION(Container, "Container")
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
		CheckException<E>((bool)ptr, message);
	}
}