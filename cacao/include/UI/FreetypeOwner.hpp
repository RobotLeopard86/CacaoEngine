#pragma once

#include "ft2build.h"
#include FT_FREETYPE_H

//Quick convienience shortcut
#define ftLib FreetypeOwner::GetInstance()->GetLib()

namespace Cacao {
	/**
	 * @brief Manages the FreeType instance
	 */
	class FreetypeOwner {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static FreetypeOwner* GetInstance();

		/**
		 * @brief Initialize FreeType
		 *
		 * @throws Exception If FreeType was already initialized or fails to initialize
		 */
		void Init();

		/**
		 * @brief Get the FreeType library instance
		 *
		 * @return The FreeType instance
		 */
		FT_Library& GetLib() {
			return lib;
		}

		/**
		 * @brief Destroys the object and the FreeType instance if it exists
		 */
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
