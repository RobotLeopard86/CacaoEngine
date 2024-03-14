#include "Graphics/Rendering/RenderController.hpp"

#include "glad/gl.h"
#include "GLFW/glfw3.h"

#include "Graphics/Window.hpp"

namespace Cacao {
	void RenderController::Render(Frame frame){
		//Clear the screen
		glClearColor(0.765625f, 1.0f, 0.1015625f, 1.0f); //This color is an obnoxious neon alligator green
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Render main scene
		for(RenderObject& obj : frame.objects){
			//Upload material data to shader
			obj.material.shader->UploadData(obj.material.data);
			obj.material.shader->UploadCacaoData(frame.projection, frame.view, obj.transformMatrix);

			//Bind shader
			obj.material.shader->Bind();

			//Configure OpenGL depth behavior
			glDepthFunc(GL_LESS);

			//Draw the mesh
			obj.mesh->Draw();

			//Unbind shader
			obj.material.shader->Unbind();
		}

		//Draw skybox (if one exists)
		if(frame.skybox.has_value()) frame.skybox.value().Draw(frame.projection, frame.view);
	}

	void RenderController::Init() {
		//Take control of the OpenGL context
		glfwMakeContextCurrent((GLFWwindow*)Window::GetInstance()->GetNativeWindow());
	}
}