#pragma once

#include "Utilities/Asset.hpp"

#include <future>

namespace Cacao {
	/**
	 * @brief Base texture asset
	 */
	class Texture : public Asset {
	  public:
		/**
		 * @brief Attach to the specified slot
		 *
		 * @param slot The texture slot to attach to
		 *
		 * @note For use by the engine only
		 *
		 * @throw Exception If texture is already bound, not compiled, or if not called on the main thread
		 */
		virtual void Bind(int slot) {}

		/**
		 * @brief Detach from the current slot
		 *
		 * @note For use by the engine only
		 *
		 * @throw Exception If texture is already bound, not compiled, or if not called on the main thread
		 */
		virtual void Unbind() {}

		/**
		 * @brief Check if the texture is bound
		 *
		 * @return Whether the texture is bound or not
		 */
		bool IsBound() const {
			return bound;
		}

	  protected:
		int currentSlot;

		bool bound;

		//Constructor for initialization purposes
		Texture(bool initialState)
		  : Asset(initialState), bound(false) {}
	};
}