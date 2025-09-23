#include "UI/Font.hpp"

#include "Core/Exception.hpp"
#include "Core/Engine.hpp"
#include "UI/FreetypeOwner.hpp"

#include <filesystem>

namespace Cacao {

	Font::Font(std::string path)
	  : Asset(false) {
		CheckException(std::filesystem::exists(path), Exception::GetExceptionCodeFromMeaning("FileNotFound"), "Cannot load font from nonexistent file!");
		filePath = path;
	}

	std::shared_future<void> Font::CompileAsync() {
		CheckException(!compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot compile compiled font!");
		return Engine::Get()->GetThreadPool()->enqueue([this]() { this->CompileSync(); }).share();
	}

	void Font::CompileSync() {
		CheckException(!compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot compile compiled font!");

		//Load FreeType font face
		CheckException(!FT_New_Face(ftLib, filePath.c_str(), 0, &face), Exception::GetExceptionCodeFromMeaning("IO"), "Failed to load font face!");

		compiled = true;
	}

	void Font::Release() {
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot release uncompiled font!");

		//Destroy FreeType font face
		FT_Done_Face(face);

		compiled = false;
	}
}