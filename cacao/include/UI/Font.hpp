#pragma once

#include "Utilities/Asset.hpp"

#include "hb.h"
#include "ft2build.h"
#include FT_FREETYPE_H

namespace Cacao {
	//A font face
	class Font final : public Asset {
	  public:
		Font(std::string path);

		~Font() final {
			if(compiled) Release();
		}

		//Compile font to be used later
		std::shared_future<void> Compile() override;

		//Delete compiled data when no longer needed
		void Release() override;

		///@brief Gets the type of this asset. Needed for safe downcasting from Asset
		std::string GetType() override {
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