#pragma once

#include "DllHelper.hpp"

#include <string>

namespace Cacao {
	/**
	 * @brief Informational struct with some info about your game
	 */
	struct CACAO_API ClientIdentity {
		std::string id;			///<ID in reverse-domain format (not enforced)
		std::string displayName;///<Display name for game
	};
}