#pragma once

#include "glm/vec2.hpp"

namespace Cacao {
	/**
	 * @brief An encapsulation of image data and the values needed to interpret it
	 */
	struct RawImage {
		unsigned char* dataBuffer;///<The actual buffer for the image data
		glm::uvec2 imgSize;		  ///<The size of the image in pixels
		uint8_t numImgChannels;	  ///<The number of channels in the image (1=Grayscale, 3=RGB, 4=RGBA)
	};
}