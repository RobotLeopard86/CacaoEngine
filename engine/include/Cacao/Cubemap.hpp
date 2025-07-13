#pragma once

#include "DllHelper.hpp"
#include "Asset.hpp"

#include "libcacaoformats.hpp"

namespace Cacao {
	/**
	 * @brief Asset type for 3D cube textures
	 */
	class CACAO_API Cubemap final : public Asset {
	  public:
		///@cond
		Cubemap(const Cubemap&) = delete;
		Cubemap(Cubemap&&);
		Cubemap& operator=(const Cubemap&) = delete;
		Cubemap& operator=(Cubemap&&);
		///@endcond

		/**
		 * @brief Synchronously convert the image data into a form suitable for rendering
		 *
		 * @throws BadRealizeStateException If the cubemap is already realized
		 * @throws BadInitStateException If the graphics backend is not initialized or connected
		 */
		void Realize();

		/**
		 * @brief Asynchronously convert the image data into a form suitable for rendering
		 *
		 * @return A future that will resolve when realization is complete or fails
		 *
		 * @throws BadRealizeStateException If the cubemap is already realized
		 * @throws BadInitStateException If the graphics backend is not initialized or connected
		 */
		std::shared_future<void> RealizeAsync();

		/**
		 * @brief Destroy the realized representation of the asset
		 *
		 * @throws BadRealizeStateException If the cubemap is not realized
		 * @throws BadInitStateException If the graphics backend is not initialized or connected
		 */
		void DropRealized();

		///@cond
		struct Impl;
		///@endcond

		~Cubemap();

	  private:
		/**
		 * @brief Create a new cubemap from image data
		 *
		 * @note This constructor must be called indirectly via ResourceManager::Instantiate
		 *
		 * @param faces The face images of the cubemap, in the order of +X face, -X face, +Y face, -Y face, +Z face, -Z face
		 * @param addr The resource address identifier to associate with the cubemap
		 */
		Cubemap(std::array<libcacaoformats::ImageBuffer, 6>&& faces, const std::string& addr);
		friend class ResourceManager;

		std::unique_ptr<Impl> impl;
		friend class ImplAccessor;

		std::array<libcacaoformats::ImageBuffer, 6> img;
	};
}