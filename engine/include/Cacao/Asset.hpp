#pragma once

#include "DllHelper.hpp"
#include "Resource.hpp"

namespace Cacao {
	/**
	 * @brief Base class for asset types
	 */
	class CACAO_API Asset : public Resource {
	  public:
		/**
		 * @brief Convert the loaded data into a form suitable for use
		 *
		 * @throws BadBakeStateException If the asset is already baked
		 */
		virtual void Bake() = 0;

		/**
		 * @brief Destroy the baked representation of the asset
		 *
		 * @throws BadBakeStateException If the asset is not baked
		 */
		virtual void Discard() = 0;

		/**
		 * @brief Check if the asset is baked
		 *
		 * @return Whether the asset is baked
		 */
		virtual bool IsBaked() const {
			return baked;
		}

		virtual ~Asset() {}

	  protected:
		Asset(const std::string& addr)
		  : Resource(addr), baked(false) {}

		bool baked;
	};
}