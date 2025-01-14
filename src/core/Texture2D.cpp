#include "Graphics/Textures/Texture2D.hpp"

#include "Core/Exception.hpp"

#include <cstdlib>

namespace Cacao {
	Texture2D::Texture2D(const RawImage& raw)
	  : Texture(false) {
		CheckException(raw.dataBuffer, Exception::GetExceptionCodeFromMeaning("IO"), "Cannot create 2D texture from nonexistent data source!");

		//Load image
		std::size_t dataSize = raw.imgSize.x * raw.imgSize.y * raw.numImgChannels;
		dataBuffer = (unsigned char*)malloc(dataSize);
		std::memcpy(dataBuffer, raw.dataBuffer, dataSize);
		imgSize = raw.imgSize;
		numImgChannels = raw.numImgChannels;

		bound = false;
		currentSlot = -1;

		//Do backend initialization
		_BackendInit();
	}
}
