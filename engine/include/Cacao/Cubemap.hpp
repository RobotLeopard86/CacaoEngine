#pragma once

#include "DllHelper.hpp"
#include "Asset.hpp"

#include "libcacaoimage.hpp"

#include <memory>
#include <array>

namespace Cacao {
	/**
	 * @brief Asset type for 3D cube textures
	 */
	class CACAO_API Cubemap final : public Asset {
	  public:
		/**
		 * @brief Create a new cubemap from image data
		 *
		 * @param faces The face images of the cubemap, in the order of +X face, -X face, +Y face, -Y face, +Z face, -Z face
		 * @param addr The resource address to associate with the cubemap
		 *
		 * @throws BadValueException If one of the faces is not in the RGB layout
		 * @throws BadValueException If the faces are not all the same size
		 * @throws BadValueException If the address is malformed
		 */
		static std::shared_ptr<Cubemap> Create(std::array<libcacaoimage::Image, 6>&& faces, const std::string& addr);

		///@cond
		Cubemap(const Cubemap&) = delete;
		Cubemap(Cubemap&&);
		Cubemap& operator=(const Cubemap&) = delete;
		Cubemap& operator=(Cubemap&&);
		///@endcond

		/**
		 * @brief Convert the image data into a form suitable for rendering
		 *
		 * @throws BadBakeStateException If the cubemap is already baked
		 * @throws BadInitStateException If the graphics backend is not initialized or connected
		 */
		void Bake() override;

		/**
		 * @brief Destroy the baked representation of the asset
		 *
		 * @throws BadBakeStateException If the cubemap is not baked
		 * @throws BadInitStateException If the graphics backend is not initialized or connected
		 */
		void Discard() override;

		///@cond
		class Impl;
		///@endcond

		~Cubemap();

	  private:
		Cubemap(std::array<libcacaoimage::Image, 6>&& faces, const std::string& addr);
		friend class PAL;
		friend class ResourceManager;

		std::unique_ptr<Impl> impl;
		friend class ImplAccessor;
	};
}