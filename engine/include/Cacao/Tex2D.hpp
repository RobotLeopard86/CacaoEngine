#pragma once

#include "DllHelper.hpp"
#include "Asset.hpp"

#include "libcacaoimage.hpp"

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
		 * @param addr The resource address to associate with the texture
		 *
		 * @throws BadValueException If the image buffer is empty
		 * @throws BadValueException If the address is malformed
		 */
		static std::shared_ptr<Tex2D> Create(libcacaoimage::Image&& imageBuffer, const std::string& addr);

		///@cond
		Tex2D(const Tex2D&) = delete;
		Tex2D(Tex2D&&);
		Tex2D& operator=(const Tex2D&) = delete;
		Tex2D& operator=(Tex2D&&);
		///@endcond

		/**
		 * @brief Convert the image data into a form suitable for rendering
		 *
		 * @throws BadBakeStateException If the texture is already baked
		 * @throws BadInitStateException If the graphics backend is not initialized or connected
		 */
		void Bake();

		/**
		 * @brief Destroy the baked representation of the asset
		 *
		 * @throws BadBakeStateException If the texture is not baked
		 * @throws BadInitStateException If the graphics backend is not initialized or connected
		 */
		void Discard();

		///@cond
		class Impl;
		///@endcond

		~Tex2D();

	  private:
		Tex2D(libcacaoimage::Image&& imageBuffer, const std::string& addr);
		friend class PAL;
		friend class ResourceManager;

		std::unique_ptr<Impl> impl;
		friend class ImplAccessor;
	};
}