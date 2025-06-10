#pragma once

#include "DllHelper.hpp"

#include <string>
#include <memory>

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
		ResourceTracker(const std::string& trackingAddress);

		/**
		 * @brief Access the underlying resource
		 */
		std::shared_ptr<T> operator->() {
			//still need to figure out this mechanism, this is a dummy placeholder
			return std::shared_ptr<T>(nullptr);
		}

	  private:
		std::string track;
	};
}