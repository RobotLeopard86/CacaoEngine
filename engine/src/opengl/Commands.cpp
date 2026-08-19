#include "Cacao/Exceptions.hpp"
#include "Context.hpp"
#include "OpenGLModule.hpp"
#include "ImplAccessor.hpp"
#include "impls/OpenGLMaterial.hpp"
#include "impls/OpenGLMesh.hpp"

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
		AddTask([this, mesh, material, transform]() -> void {
			Check<Mesh, NonexistentValueException>(mesh, "Cannot draw null mesh!");
			Check<Material, NonexistentValueException>(material, "Cannot draw mesh with null material!");
			Check<BadBakeStateException>(mesh->IsBaked(), "Cannot draw unbaked mesh!");
			Check<BadBakeStateException>(material->GetShader()->IsBaked(), "Cannot draw mesh using unbaked shader!");

			//Apply material (will also bind shader pipeline)
			RES_IMPL(Material, OpenGL, *material).Apply(this);

			//Bind mesh vertex and index buffer
			OpenGLMeshImpl& glMesh = RES_IMPL(Mesh, OpenGL, *mesh);
			glBindVertexArray(glMesh.vao);
		});
	}

	void OpenGLCommandBuffer::UpdateEngineData(std::shared_ptr<Camera> cam, bool worldRefresh) {
		AddTask([cam, worldRefresh]() {
			//TODO
		});
	}
}