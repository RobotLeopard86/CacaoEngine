#pragma once

#include "DllHelper.hpp"

#include <string>
#include <vector>

namespace Cacao {
	/**
	 * @brief Base class for any game-related resource (world, asset, arbitrary blob, etc.)
	 */
	class CACAO_API Resource {
	  public:
		/**
		 * @brief Get the resource's address (used to reference it)
		 *
		 * @return The address
		 */
		std::string GetAddress() {
			return address;
		}

		virtual ~Resource() {}

	  protected:
		Resource(const std::string& addr)
		  : address(addr) {}

		const std::string address;
	};

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