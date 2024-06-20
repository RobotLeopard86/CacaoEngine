#pragma once

#include "Utilities/MiscUtils.hpp"
#include "ESUtils.hpp"

#include "glad/gles2.h"

namespace Cacao {
	//Struct for data required for an OpenGL ES skybox
	struct ESSkyboxData : public NativeData {
		GLuint vao, vbo;
		bool vaoReady;

		~ESSkyboxData() {
			if(vaoReady) {
				//Copy VAO and VBO names so they can be deleted even if object is first
				GLuint vertexArray = vao, vertexBuffer = vbo;
				GLJob job([vertexArray, vertexBuffer]() {
					glDeleteBuffers(1, &vertexBuffer);
					glDeleteVertexArrays(1, &vertexArray);
				});
				EnqueueGLJob(job);
			}
		}
	};
}