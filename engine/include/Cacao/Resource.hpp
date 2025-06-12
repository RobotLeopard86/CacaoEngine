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

	//This is here because otherwise we'd end up with a cyclical header dependency
	///@cond
	std::shared_ptr<Resource> GetRPtr(const std::string& addr);
	///@endcond

	/**
	 * @brief A wrapper class that exists to track the state of resources in the overlay stack so that when changes occur, they are automatically reflected.
	 */
	template<typename T>
		requires std::is_base_of_v<Resource, T>
	class CACAO_API ResourceHandle {
	  public:
		/**
		 * @brief Create a forwarder tracking a given resource address
		 *
		 * @param trackingAddress The resource address to track
		 *
		 * @throws BadValueException If the tracking address refers to a component
		 */
		ResourceHandle(const std::string& trackingAddress);

		/**
		 * @brief Access the underlying resource
		 */
		std::shared_ptr<T> operator->() {
			//Get pointer from resource manager to check for update
			std::shared_ptr<Resource> rptr = GetRPtr(track);
			std::shared_ptr<T> ptr = std::dynamic_pointer_cast<T>(rptr);
			Check<T, BadTypeException>(ptr, "Resource handle-tracked resource's type does not match handle type!");

			//Update handle to keep ownership if needed
			if(ptr != handle) handle = ptr;

			return handle;
		}

	  private:
		std::string track;
		std::shared_ptr<T> handle;
	};
}