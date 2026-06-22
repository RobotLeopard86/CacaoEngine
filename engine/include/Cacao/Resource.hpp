#pragma once

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
	class CACAO_API Resource : public std::enable_shared_from_this<Resource> {
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
			requires std::is_base_of_v<Resource, T> && (!std::is_same_v<Asset, T>)
		static bool ValidateResourceAddr(const std::string& addr);

		virtual ~Resource();

	  protected:
		Resource(const std::string& addr)
		  : address(addr) {}

		std::string address;
	};

	/**
	 * @brief Resource that holds a blob of data
	 */
	class CACAO_API BlobResource : public Resource {
	  public:
		/**
		 * @brief Create a new blob from binary data
		 *
		 * @param data The data blob to store
		 * @param addr The resource address to associate with the blob
		 *
		 * @throws BadValueException If the address is malformed
		 */
		static std::shared_ptr<BlobResource> Create(std::vector<unsigned char>&& data, const std::string& addr) {
			return std::shared_ptr<BlobResource>(new BlobResource(std::move(data), addr));
		}

		/**
		 * @brief Create a new blob from textual data
		 *
		 * @param data The string to store
		 * @param addr The resource address to associate with the blob
		 *
		 * @throws BadValueException If the address is malformed
		 */
		static std::shared_ptr<BlobResource> Create(std::string&& data, const std::string& addr) {
			return std::shared_ptr<BlobResource>(new BlobResource(std::move(data), addr));
		}

		/**
		 * @brief Access the stored data
		 *
		 * @return A constant reference to the data
		 */
		const std::vector<unsigned char>& GetData() {
			return data;
		}

		/**
		 * @brief Access the stored data as a string
		 *
		 * @return A constant reference to the data
		 */
		std::string_view GetDataAsString() {
			return std::string_view(reinterpret_cast<const char*>(data.data()), data.size());
		}

	  private:
		BlobResource(std::vector<unsigned char>&& data, const std::string& addr);
		BlobResource(std::string&& data, const std::string& addr);

		const std::vector<unsigned char> data;

		friend class ResourceManager;
	};
}