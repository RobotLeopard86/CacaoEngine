#pragma once

#include "Exception.hpp"

#include "yaml-cpp/yaml.h"

namespace Cacao {
	inline void CheckException(YAML::Node node, unsigned int exceptionCode, std::string exceptionDescription) {
		if(!(bool)node) {
			Exception ex {exceptionDescription, exceptionCode};
			Logging::EngineLog(ex.what(), LogLevel::Error);
			throw ex;
		}
	}
}