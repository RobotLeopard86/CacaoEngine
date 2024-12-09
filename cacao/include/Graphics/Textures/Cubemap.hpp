#pragma once

#include "Texture.hpp"
#include "Utilities/MiscUtils.hpp"

#include "glm/vec2.hpp"

#include <string>
#include <vector>

namespace Cacao {
	/**
	 * @brief Cube texture
	 */
	class Cubemap final : public Texture {
	  public:
		/**
		 * @brief Create a new cubemap from a file list
		 *
		 * @param filePaths The paths to image files for each face. Must be in the order +X, -X, +Y, -Y, +Z, -Z
		 *
		 * @note Prefer to use AssetManager::LoadCubemap over direct construction
		 *
		 * @throws Exception If any of the specified files does not exist
		 */
		Cubemap(std::vector<std::string> filePaths);

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
		 * @throw Exception If cubemap is already bound, not compiled, or if not called on the main thread
		 */
		void Bind(int slot) override;

		/**
		 * @brief Detach from the current slot
		 *
		 * @note For use by the engine only
		 *
		 * @throw Exception If cubemap is already bound, not compiled, or if not called on the main thread
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

		std::vector<std::string> textures;

		std::shared_ptr<CubemapData> nativeData;
	};
}