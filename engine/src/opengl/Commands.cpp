#include "BackendCommon.hpp"
#include "Cacao/Exceptions.hpp"
#include "Context.hpp"
#include "OpenGLModule.hpp"
#include "ImplAccessor.hpp"
#include "impls/OpenGLMaterial.hpp"
#include "impls/OpenGLMesh.hpp"
#include "impls/OpenGLShader.hpp"

#include "glad/gl.h"
#include "glm/gtc/type_ptr.hpp"

#include <memory>

namespace Cacao {
	void OpenGLCommandBuffer::StartRendering(glm::vec3 clearColor) {
		AddTask([clearColor]() {
			//Clear screen
			glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0);
			glClearDepth(1.0);
			glDepthMask(GL_TRUE);
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

			//Bind mesh vertex array
			OpenGLMeshImpl& glMesh = RES_IMPL(Mesh, OpenGL, *mesh);
			glBindVertexArray(glMesh.vao);

			//Upload uniforms
			OpenGLShaderImpl& glShader = RES_IMPL(Shader, OpenGL, *(material->GetShader()));
			glm::mat4 transformationMatrix = transform.GetTransformationMatrix();
			if(glShader.transformUloc != -1) glUniformMatrix4fv(glShader.transformUloc, 1, GL_TRUE, glm::value_ptr(transformationMatrix));
			if(glShader.normalMatrixUloc != -1 || glShader.handednessUloc != -1) {
				glm::mat3 transformLinear(transformationMatrix);
				glm::mat3 normalMatrix = glm::transpose(glm::inverse(transformLinear));
				if(glShader.normalMatrixUloc != -1) glUniformMatrix3fv(glShader.normalMatrixUloc, 1, GL_TRUE, glm::value_ptr(normalMatrix));
				if(glShader.handednessUloc != -1) glUniform1f(glShader.handednessUloc, (glm::determinant(transformLinear) < 0.0f ? -1.0f : 1.0f));
			}
			glUniform1ui(glShader.renderModeUloc, static_cast<uint8_t>(material->GetRenderMode()));

			//Draw the mesh
			glDrawElements(GL_TRIANGLES, glMesh.indices.size() * 3, GL_UNSIGNED_INT, nullptr);

			//Rese state
			glBindVertexArray(0);
			glUseProgram(0);
		});
	}

	void OpenGLCommandBuffer::UpdateEngineData(std::shared_ptr<Camera> cam, bool worldRefresh) {
		AddTask([cam, worldRefresh]() {
			//Camera info
			GlobalsData gd = {};
			gd.viewMatrix = cam->GetViewMatrix();
			gd.projectionMatrix = cam->GetProjectionMatrix();
			gd.viewProjectionMatrix = gd.projectionMatrix * gd.viewMatrix;
			gd.camWorldRot = cam->GetRotation();
			gd.camWorldPos = cam->GetPosition();

			//Timing info
			using clock = std::chrono::steady_clock;
			static clock::time_point initial = clock::now();
			static clock::time_point last = clock::now();
			clock::time_point now = clock::now();
			if(worldRefresh) {
				initial = now;
				last = now;
			}
			gd.deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(now - last).count();
			gd.worldTime = std::chrono::duration_cast<std::chrono::duration<float>>(now - initial).count();
			if(!worldRefresh) last = now;

			//Bind UBO
			glBindBuffer(GL_UNIFORM_BUFFER, gl->globalsUBO);

			//Upload data
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GlobalsData), &gd);

			//Unbind UBO
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		});
	}
}