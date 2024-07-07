#pragma once

#include "ft2build.h"
#include FT_FREETYPE_H

//Quick convienience shortcut
#define ftLib FreetypeOwner::GetInstance()->GetLib()

namespace Cacao {
	//Owns the FreeType library instance
	class FreetypeOwner {
	  public:
		//Get the instance or create one if it doesn't exist.
		static FreetypeOwner* GetInstance();

		//Get the library instance
		FT_Library& GetLib() {
			return lib;
		}

	  private:
		//Singleton members
		static FreetypeOwner* instance;
		static bool instanceExists;

		//Library instance
		//Created when this object is and destroyed when this object is
		FT_Library lib;

		FreetypeOwner();
	};
}
