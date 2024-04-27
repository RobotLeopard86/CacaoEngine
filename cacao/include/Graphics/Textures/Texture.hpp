#pragma once

#include "Utilities/Asset.hpp"

#include <future>

namespace Cacao {
	//Base texture type
	class Texture : public Asset {
	  public:
		//Attach this texture to the specified slot
		virtual void Bind(int slot) {}
		//Detach this texture
		virtual void Unbind() {}

		//Is texture bound?
		bool IsBound() const {
			return bound;
		}

	  protected:
		int currentSlot;

		bool bound;

		//Constructor for initialization purposes
		Texture(bool initiallyCompiled)
		  : Asset(initiallyCompiled) {}
	};
}