#include "UI/Font.hpp"

#include "Core/Exception.hpp"
#include "Core/Engine.hpp"
#include "FreetypeOwner.hpp"

#include "hb.h"

#include <filesystem>

namespace Cacao {

	Font::Font(unsigned char* fontData, long dataSize)
	  : Asset(false) {
		CheckException(fontData, Exception::GetExceptionCodeFromMeaning("NullValue"), "Cannot load font from null data!");
		CheckException(dataSize > 0, Exception::GetExceptionCodeFromMeaning("BadValue"), "Cannot load font with bad data size!");
		raw = new unsigned char[dataSize];
		std::memcpy(raw, fontData, dataSize);
		rawDataSize = dataSize;
	}

	std::shared_future<void> Font::CompileAsync() {
		CheckException(!compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot compile compiled font!");
		return Engine::GetInstance()->GetThreadPool()->enqueue([this]() { this->CompileSync(); }).share();
	}

	void Font::CompileSync() {
		CheckException(!compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot compile compiled font!");

		//Load FreeType font face
		CheckException(!FT_New_Memory_Face(ftLib, raw, rawDataSize, 0, &face), Exception::GetExceptionCodeFromMeaning("External"), "Failed to create font face!");

		compiled = true;
	}

	void Font::Release() {
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot release uncompiled font!");

		//Destroy FreeType font face
		FT_Done_Face(face);

		compiled = false;
	}
}