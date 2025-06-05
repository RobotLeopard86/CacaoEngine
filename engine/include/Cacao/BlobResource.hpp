#pragma once

#include "DllHelper.hpp"
#include "Resource.hpp"

#include <vector>

namespace Cacao {
	/**
	 * @brief The base type for any game-defined resource types loaded from data blobs
	 */
	class CACAO_API BlobResource : public Resource {
	  protected:
		BlobResource(const std::string& addr, std::vector<unsigned char>&& data)
		  : Resource(addr), data(data) {}

		const std::vector<unsigned char> data;
	};
}