#include "Context.hpp"
#include "OpenGLModule.hpp"

#include "glad/gl.h"
#include <memory>

namespace Cacao {
	void OpenGLCommandBuffer::StartRendering(glm::vec3 clearColor) {
		AddTask([clearColor]() {
			//Clear screen
			glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		});
	}

	void OpenGLCommandBuffer::EndRendering() {
		AddTask([]() {
			//Present (OpenGL has no formal end rendering system)
			ctx->SwapBuffers();
		});
	}

	void OpenGLCommandBuffer::DrawMesh(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material, Transform transform) {
		AddTask([mesh, material, transform]() {
			//TODO
		});
	}

	void OpenGLCommandBuffer::UpdateEngineData(std::shared_ptr<Camera> cam, bool worldRefresh) {
		AddTask([cam, worldRefresh]() {
			//TODO
		});
	}
}