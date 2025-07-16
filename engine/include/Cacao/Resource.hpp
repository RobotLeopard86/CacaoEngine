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
		const std::string GetAddress() const {
			return address;
		}

		virtual ~Resource();

	  protected:
		Resource(const std::string& addr)
		  : address(addr) {}

		std::string address;
	};

	/**
	 * @brief Base class for resource blobs (it does functionally nothing but it's for grouping purposes)
	 */
	class CACAO_API BlobResource : public Resource {
	  protected:
		BlobResource(const std::string& addr)
		  : Resource(addr) {}
	};

	/**
	 * @brief The resource type for binary data blobs
	 */
	class CACAO_API BinaryBlobResource : public BlobResource {
	  private:
		BinaryBlobResource(const std::string& addr, std::vector<unsigned char>&& data)
		  : BlobResource(addr), data(data) {}

		const std::vector<unsigned char> data;

		friend class ResourceManager;
	};

	/**
	 * @brief The resource type for data blobs containing text
	 */
	class CACAO_API TextBlobResource : public BlobResource {
	  private:
		TextBlobResource(const std::string& addr, std::string data)
		  : BlobResource(addr), data(data) {}

		const std::string data;

		friend class ResourceManager;
	};
}