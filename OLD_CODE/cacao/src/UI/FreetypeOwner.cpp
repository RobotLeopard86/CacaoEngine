#include "UI/FreetypeOwner.hpp"

#include "Core/Exception.hpp"

namespace Cacao {
	//Required static variable initialization
	FreetypeOwner* FreetypeOwner::instance = nullptr;
	bool FreetypeOwner::instanceExists = false;

	//Singleton accessor
	FreetypeOwner* FreetypeOwner::GetInstance() {
		//Do we have an instance yet?
		if(!instanceExists || instance == nullptr) {
			//Create instance
			instance = new FreetypeOwner();
			instanceExists = true;
		}

		return instance;
	}

	FreetypeOwner::FreetypeOwner()
	  : didInit(false) {}

	FreetypeOwner::~FreetypeOwner() {
		if(didInit) FT_Done_FreeType(lib);
	}

	void FreetypeOwner::Init() {
		CheckException(!didInit, Exception::GetExceptionCodeFromMeaning("BadInitState"), "FreeType is already initialized!");
		CheckException(!FT_Init_FreeType(&lib), Exception::GetExceptionCodeFromMeaning("External"), "Failed to initialize FreeType!");

		didInit = true;
	}
}
