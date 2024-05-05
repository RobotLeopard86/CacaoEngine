#pragma once

#include "Utilities/MiscUtils.hpp"
#include "GLUtils.hpp"

#include "glad/gl.h"

namespace Cacao {
	//Struct for data required for an OpenGL skybox
	struct GLSkyboxData : public NativeData {
		GLuint vao, vbo;
		bool vaoReady;

		~GLSkyboxData() {
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