#pragma once

#include "DllHelper.hpp"

#include <memory>
#include <string>
#include <vector>

namespace Cacao {
	/**
	 * @brief Base class for any game-related resource (world, asset, arbitrary blob, etc.)
	 */
	class CACAO_API Resource : std::enable_shared_from_this<Resource> {
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
		  : address(addr) {
			RegisterSelf();
		}

		std::string address;

		void RegisterSelf();
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
	  public:
		/**
		 * @brief Create a new blob from data
		 *
		 * @param data The data blob to store
		 * @param addr The resource address identifier to associate with the blob
		 */
		static std::shared_ptr<BinaryBlobResource> Create(std::vector<unsigned char>&& data, const std::string& addr) {
			return std::make_shared<BinaryBlobResource>(data, addr);
		}

		/**
		 * @brief Access the stored data
		 *
		 * @return A constant reference to the data
		 */
		const std::vector<unsigned char>& GetData() {
			return data;
		}

	  private:
		BinaryBlobResource(std::vector<unsigned char>&& data, const std::string& addr)
		  : BlobResource(addr), data(data) {}

		const std::vector<unsigned char> data;

		friend class ResourceManager;
	};

	/**
	 * @brief The resource type for data blobs containing text
	 */
	class CACAO_API TextBlobResource : public BlobResource {
	  public:
		/**
		 * @brief Create a new blob from text
		 *
		 * @param data The text blob to store
		 * @param addr The resource address identifier to associate with the blob
		 */
		static std::shared_ptr<BinaryBlobResource> Create(std::string&& data, const std::string& addr) {
			return std::make_shared<BinaryBlobResource>(data, addr);
		}

		/**
		 * @brief Access the stored data
		 *
		 * @return A constant reference to the data
		 */
		const std::string& GetData() {
			return data;
		}

	  private:
		TextBlobResource(std::string&& data, const std::string& addr)
		  : BlobResource(addr), data(data) {}

		const std::string data;

		friend class ResourceManager;
	};
}