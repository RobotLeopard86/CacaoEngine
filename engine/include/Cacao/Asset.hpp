#pragma once

#include "DllHelper.hpp"

#include <future>

namespace Cacao {
	/**
	 * @brief Base class for asset types
	 */
	class CACAO_API Asset {
	  public:
		/**
		 * @brief Synchronously convert the loaded data into a form suitable for playback
		 *
		 * @throws BadRealizeStateException If the asset is already realized
		 */
		virtual void Realize() = 0;

		/**
		 * @brief Asynchronously convert the loaded data into a form suitable for playback
		 *
		 * @return A future that will resolve when realization is complete or fails
		 *
		 * @throws BadRealizeStateException If the asset is already realized
		 */
		virtual std::shared_future<void> RealizeAsync() = 0;

		/**
		 * @brief Destroy the realized representation of the asset
		 *
		 * @throws BadRealizeStateException If the asset is not realized
		 */
		virtual void DropRealized() = 0;

		/**
		 * @brief Check if the asset is realized
		 *
		 * @return Whether the asset is realized
		 */
		virtual bool IsRealized() const {
			return realized;
		}

		virtual ~Asset() {}

	  protected:
		Asset()
		  : realized(false) {}

		bool realized;
	};
}