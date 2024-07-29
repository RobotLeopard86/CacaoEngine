#include "UI/Font.hpp"

#include "Core/Exception.hpp"
#include "UI/FreetypeOwner.hpp"

#include <filesystem>

namespace Cacao {

	Font::Font(std::string path)
	  : Asset(false) {
		CheckException(std::filesystem::exists(path), Exception::GetExceptionCodeFromMeaning("FileNotFound"), "Cannot load font from nonexistent file!")
		filePath = path;
	}

	std::shared_future<void> Font::Compile() {
		CheckException(!compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot compile compiled font!")

		//Load FreeType font face
		CheckException(!FT_New_Face(ftLib, filePath.c_str(), 0, &face), Exception::GetExceptionCodeFromMeaning("IO"), "Failed to load font face!")

		compiled = true;

		//Return an already completed future
		std::promise<void> p;
		p.set_value();
		return p.get_future().share();
	}

	void Font::Release() {
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot release uncompiled font!")

		//Destroy FreeType font face
		FT_Done_Face(face);

		compiled = false;
	}
}