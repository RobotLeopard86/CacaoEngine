#pragma once

#include "Cacao/Exceptions.hpp"
#include "DllHelper.hpp"

#include <string>
#include <memory>
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

		virtual ~Resource();

	  protected:
		Resource(const std::string& addr, const std::string& pkg)
		  : address(addr), pkg(pkg) {}

		const std::string address;
		const std::string pkg;
	};

	/**
	 * @brief The base type for any game-defined resource types loaded from data blobs
	 */
	class CACAO_API BlobResource : public Resource {
	  protected:
		BlobResource(const std::string& addr, const std::string& pkg, std::vector<unsigned char>&& data)
		  : Resource(addr, pkg), data(data) {}

		const std::vector<unsigned char> data;
	};
}