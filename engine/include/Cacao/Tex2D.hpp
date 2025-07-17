#pragma once

#include "DllHelper.hpp"
#include "Asset.hpp"

#include "libcacaoformats.hpp"

#include <memory>

namespace Cacao {
	/**
	 * @brief Asset type for 2D textures
	 */
	class CACAO_API Tex2D final : public Asset {
	  public:
		/**
		 * @brief Create a new 2D texture from image data
		 *
		 * @param imageBuffer The image data for the texture
		 * @param addr The resource address identifier to associate with the texture
		 */
		static std::shared_ptr<Tex2D> Create(libcacaoformats::ImageBuffer&& imageBuffer, const std::string& addr) {
			return std::make_shared<Tex2D>(imageBuffer, addr);
		}

		///@cond
		Tex2D(const Tex2D&) = delete;
		Tex2D(Tex2D&&);
		Tex2D& operator=(const Tex2D&) = delete;
		Tex2D& operator=(Tex2D&&);
		///@endcond

		/**
		 * @brief Synchronously convert the image data into a form suitable for rendering
		 *
		 * @throws BadRealizeStateException If the texture is already realized
		 * @throws BadInitStateException If the graphics backend is not initialized or connected
		 */
		void Realize();

		/**
		 * @brief Asynchronously convert the image data into a form suitable for rendering
		 *
		 * @return A future that will resolve when realization is complete or fails
		 *
		 * @throws BadRealizeStateException If the texture is already realized
		 * @throws BadInitStateException If the graphics backend is not initialized or connected
		 */
		std::shared_future<void> RealizeAsync();

		/**
		 * @brief Destroy the realized representation of the asset
		 *
		 * @throws BadRealizeStateException If the texture is not realized
		 * @throws BadInitStateException If the graphics backend is not initialized or connected
		 */
		void DropRealized();

		///@cond
		struct Impl;
		///@endcond

		~Tex2D();

	  private:
		Tex2D(libcacaoformats::ImageBuffer&& imageBuffer, const std::string& addr);
		friend class ResourceManager;

		std::unique_ptr<Impl> impl;
		friend class ImplAccessor;

		libcacaoformats::ImageBuffer img;
	};
}