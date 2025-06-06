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

	/**
	 * @brief A wrapper class that exists to track the state of resources in the overlay stack so that when changes occur, they are automatically reflected.
	 */
	template<typename T>
		requires std::is_base_of_v<Resource, T>
	class CACAO_API ResourceTracker {
	  public:
		/**
		 * @brief Create a forwarder tracking a given resource address
		 *
		 * @param trackingAddress The resource address to track
		 *
		 * @throws BadValueException If the tracking address refers to a component
		 */
		ResourceForwarder(const std::string& trackingAddress);

		/**
		 * @brief Access the underlying resource
		 */
		std::shared_ptr<T> operator->() {
		}
	};
}