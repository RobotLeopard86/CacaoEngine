#pragma once

#include "Cacao/Exceptions.hpp"
#include "DllHelper.hpp"

#include <memory>
#include <string>
#include <vector>

namespace Cacao {
	class BlobResource;
	class Asset;

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

		/**
		 * @brief Check if a resource address is valid for a particular type
		 *
		 * @param addr The address to check
		 *
		 * @return If the address is valid
		 */
		template<typename T>
			requires std::is_base_of_v<Resource, T> && (!std::is_same_v<BlobResource, T>) && (!std::is_same_v<Asset, T>)
		static bool ValidateResourceAddr(const std::string& addr);

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
	class CACAO_API BinaryBlobResource final : public BlobResource {
	  public:
		/**
		 * @brief Create a new blob from data
		 *
		 * @param data The data blob to store
		 * @param addr The resource address to associate with the blob
		 *
		 * @throws BadValueException If the address is malformed
		 */
		static std::shared_ptr<BinaryBlobResource> Create(std::vector<unsigned char>&& data, const std::string& addr) {
			return std::shared_ptr<BinaryBlobResource>(new BinaryBlobResource(std::move(data), addr));
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
		BinaryBlobResource(std::vector<unsigned char>&& data, const std::string& addr);

		const std::vector<unsigned char> data;

		friend class ResourceManager;
	};

	/**
	 * @brief The resource type for data blobs containing text
	 */
	class CACAO_API TextBlobResource final : public BlobResource {
	  public:
		/**
		 * @brief Create a new blob from text
		 *
		 * @param data The text blob to store
		 * @param addr The resource address identifier to associate with the blob
		 *
		 * @throws BadValueException If the address is malformed
		 */
		static std::shared_ptr<TextBlobResource> Create(std::string&& data, const std::string& addr) {
			return std::shared_ptr<TextBlobResource>(new TextBlobResource(std::move(data), addr));
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
		TextBlobResource(std::string&& data, const std::string& addr);

		const std::string data;

		friend class ResourceManager;
	};
}