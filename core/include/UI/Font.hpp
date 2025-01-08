#pragma once

#include "Utilities/Asset.hpp"
#include "Core/DllHelper.hpp"

//FreeType face forward declaration
///@cond
typedef struct FT_FaceRec_ FT_FaceRec_;
typedef FT_FaceRec_* FT_Face;
///@endcond

namespace Cacao {
	/**
	 * @brief A font face for text rendering
	 */
	class CACAO_API Font final : public Asset {
	  public:
		/**
		 * @brief Create a font from a file path
		 *
		 * @param path The path to a font file to load
		 *
		 * @note Prefer to use AssetManager::LoadFont over direct construction
		 */
		Font(std::string path);

		/**
		 * @brief Destroy the font face and its compiled data if applicable
		 */
		~Font() final {
			if(compiled) Release();
		}

		/**
		 * @brief Compile the raw font data into a usable font face asynchronously
		 *
		 * @return A future that will resolve when compilation is done
		 *
		 * @throws Exception If the font face was already compiled
		 */
		std::shared_future<void> CompileAsync() override;

		/**
		 * @brief Compile the raw font data into a usable font face synchronously
		 *
		 * @throws Exception If the font face was already compiled
		 */
		void CompileSync() override;

		/**
		 * @brief Delete the compiled data
		 *
		 * @throws Exception If the font face was not compiled
		 */
		void Release() override;

		///@brief Gets the type of this asset. Needed for safe downcasting from Asset
		std::string GetType() const override {
			return "FONT";
		}

	  private:
		//Path to font file
		std::string filePath;

		//FreeType font face
		FT_Face face;

		friend class Text;
	};
}