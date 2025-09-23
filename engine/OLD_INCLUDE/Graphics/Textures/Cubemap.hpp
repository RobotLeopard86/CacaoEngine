#pragma once

#include "Texture.hpp"
#include "RawImage.hpp"
#include "Utilities/MiscUtils.hpp"
#include "Core/DllHelper.hpp"

#include "glm/vec2.hpp"

#include <string>
#include <vector>

namespace Cacao {
	/**
	 * @brief Cube texture
	 */
	class CACAO_API Cubemap final : public Texture {
	  public:
		/**
		 * @brief Create a new cubemap from image data
		 *
		 * @param posX The image data for the positive X face (copied on creation)
		 * @param negX The image data for the negative X face (copied on creation)
		 * @param posY The image data for the positive Y face (copied on creation)
		 * @param negY The image data for the negative Y face (copied on creation)
		 * @param posZ The image data for the positive Z face (copied on creation)
		 * @param negZ The image data for the negative Z face (copied on creation)
		 *
		 * @note Prefer to use AssetManager::LoadCubemap over direct construction
		 *
		 */
		Cubemap(RawImage posX, RawImage negX, RawImage posY, RawImage negY, RawImage posZ, RawImage negZ);

		/**
		 * @brief Destroy the cubemap and its compiled data if applicable
		 */
		~Cubemap() final {
			if(bound) Unbind();
			if(compiled) Release();
		}

		/**
		 * @brief Attach to the specified slot
		 *
		 * @param slot The texture slot to attach to
		 *
		 * @note For use by the engine only
		 *
		 * @throw Exception If cubemap is already bound, not compiled, or if not called on the engine thread
		 */
		void Bind(int slot) override;

		/**
		 * @brief Detach from the current slot
		 *
		 * @note For use by the engine only
		 *
		 * @throw Exception If cubemap is already bound, not compiled, or if not called on the engine thread
		 */
		void Unbind() override;

		/**
		 * @brief Compile the raw image data into a format that can be sampled by the GPU asynchronously
		 *
		 * @return A future that will resolve when compilation is done
		 *
		 * @throws Exception If the cubemap was already compiled
		 */
		std::shared_future<void> CompileAsync() override;

		/**
		 * @brief Compile the raw image data into a format that can be sampled by the GPU synchronously
		 *
		 * @return A future that will resolve when compilation is done
		 *
		 * @throws Exception If the cubemap was already compiled
		 */
		void CompileSync() override;

		/**
		 * @brief Delete the compiled data
		 *
		 * @throws Exception If the cubemap was not compiled or is bound
		 */
		void Release() override;

		///@brief Gets the type of this asset. Needed for safe downcasting from Asset
		std::string GetType() const override {
			return "CUBEMAP";
		}

	  protected:
		//Backend-implemented data type
		struct CubemapData;

		RawImage px, nx, py, ny, pz, nz;

		std::shared_ptr<CubemapData> nativeData;
	};
}