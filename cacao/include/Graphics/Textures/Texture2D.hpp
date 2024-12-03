#pragma once

#include "Texture.hpp"
#include "Utilities/MiscUtils.hpp"

#include "glm/vec2.hpp"

#include <string>

namespace Cacao {
	/**
	 * @brief 2D image texture
	 */
	class Texture2D final : public Texture {
	  public:
		/**
		 * @brief Create a new texture from a file
		 *
		 * @param filePath The path to an image file
		 *
		 * @note Prefer to use AssetManager::LoadTexture2D over direct construction
		 *
		 * @throws Exception If the file does not exist or could not be opened
		 */
		Texture2D(std::string filePath);

		/**
		 * @brief Destroy the texture and its compiled data if applicable
		 */
		~Texture2D() final {
			if(bound) Unbind();
			if(compiled) Release();
			delete dataBuffer;
		}

		/**
		 * @brief Attach to the specified slot
		 *
		 * @param slot The texture slot to attach to
		 *
		 * @note For use by the engine only
		 *
		 * @throw Exception If texture is already bound, not compiled, or if not called on the main thread
		 */
		void Bind(int slot) override;

		/**
		 * @brief Detach from the current slot
		 *
		 * @note For use by the engine only
		 *
		 * @throw Exception If texture is already bound, not compiled, or if not called on the main thread
		 */
		void Unbind() override;

		/**
		 * @brief Compile the raw image data into a format that can be sampled by the GPU asynchronously
		 *
		 * @return A future that will resolve when compilation is done
		 *
		 * @throws Exception If the texture was already compiled
		 */
		std::shared_future<void> CompileAsync() override;

		/**
		 * @brief Compile the raw image data into a format that can be sampled by the GPU synchronously
		 *
		 * @return A future that will resolve when compilation is done
		 *
		 * @throws Exception If the texture was already compiled
		 */
		void CompileSync() override;

		/**
		 * @brief Delete the compiled data
		 *
		 * @throws Exception If the texture was not compiled or is bound
		 */
		void Release() override;

		///@brief Gets the type of this asset. Needed for safe downcasting from Asset
		std::string GetType() override {
			return "2DTEX";
		}

	  private:
		//Backend-implemented data type
		struct Tex2DData;

		unsigned char* dataBuffer;
		glm::ivec2 imgSize;
		int numImgChannels;

		std::shared_ptr<Tex2DData> nativeData;
	};
}