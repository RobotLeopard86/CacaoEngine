#pragma once

#include "Texture.hpp"
#include "RawImage.hpp"
#include "Utilities/MiscUtils.hpp"
#include "Core/DllHelper.hpp"

#include "glm/vec2.hpp"

#include <string>

namespace Cacao {
	/**
	 * @brief 2D image texture
	 */
	class CACAO_API Texture2D final : public Texture {
	  public:
		/**
		 * @brief Create a new texture from image data
		 *
		 * @param raw The image data, copied when imported into texture
		 *
		 * @note Prefer to use AssetManager::LoadTexture2D over direct construction
		 *
		 * @throws Exception If the file does not exist or could not be opened
		 */
		Texture2D(const RawImage& raw);

		/**
		 * @brief Destroy the texture and its compiled data if applicable
		 */
		~Texture2D() final {
			if(bound) Unbind();
			if(compiled) Release();
			free(dataBuffer);
		}

		/**
		 * @brief Attach to the specified slot
		 *
		 * @param slot The texture slot to attach to
		 *
		 * @note For use by the engine only
		 *
		 * @throw Exception If texture is already bound, not compiled, or if not called on the engine thread
		 */
		void Bind(int slot) override;

		/**
		 * @brief Detach from the current slot
		 *
		 * @note For use by the engine only
		 *
		 * @throw Exception If texture is already bound, not compiled, or if not called on the engine thread
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
		std::string GetType() const override {
			return "TEX2D";
		}

	  private:
		//Backend-implemented data type
		struct Tex2DData;

		unsigned char* dataBuffer;
		glm::uvec2 imgSize;
		uint8_t numImgChannels;

		void _BackendInit();

		std::shared_ptr<Tex2DData> nativeData;
	};
}