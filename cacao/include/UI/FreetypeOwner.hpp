#pragma once

#include "ft2build.h"
#include FT_FREETYPE_H

//Quick convienience shortcut
#define ftLib FreetypeOwner::GetInstance()->GetLib()

namespace Cacao {
	//Owns the FreeType library instance
	class FreetypeOwner {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static FreetypeOwner* GetInstance();

		//Initialize the FreeType library
		void Init();

		//Get the library instance
		FT_Library& GetLib() {
			return lib;
		}

		~FreetypeOwner();

	  private:
		//Singleton members
		static FreetypeOwner* instance;
		static bool instanceExists;

		//Library instance
		//Created when Init() is called and destroyed when this object is
		FT_Library lib;
		bool didInit;

		FreetypeOwner();
	};
}
