#include "Graphics/Rendering/RenderController.hpp"

#include "glad/gl.h"
#include "GLFW/glfw3.h"

#include "Graphics/Window.hpp"
#include "Events/EventSystem.hpp"
#include "GLUtils.hpp"

namespace Cacao {
	static float what;
	static bool lighten;

	//Queue of OpenGL jobs to process
	static std::queue<GLJob> glQueue;

	void RenderController::ProcessFrame(Frame& frame){
		//Process OpenGL jobs
		while(!glQueue.empty()){
			//Acquire next job
			GLJob& job = glQueue.front();

			//Run job
			job.func();

			//Mark job as done
			job.status->set_value();

			//Remove job from queue
			glQueue.pop();
		}

		//Clear the screen
		what += 0.01f * (lighten ? 1 : -1);
		if(what > 1 || what < 0) lighten = !lighten;
		glClearColor(0.765625f * what, 1.0f * what, 0.1015625f * what, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Render main scene
		for(RenderObject& obj : frame.objects){
			//Upload material data to shader
			obj.material.shader->UploadData(obj.material.data);
			obj.material.shader->UploadCacaoData(frame.projection, frame.view, obj.transformMatrix);

			//Bind shader
			obj.material.shader->Bind();

			//Configure OpenGL depth behavior
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);

			//Draw the mesh
			obj.mesh->Draw();

			//Unbind shader
			obj.material.shader->Unbind();
		}

		//Draw skybox (if one exists)
		if(frame.skybox.has_value()) frame.skybox.value().Draw(frame.projection, frame.view);
	}

	void EnqueueGLJob(GLJob& job) {
		glQueue.push(job);
	}

	void RenderController::Init() {
		EngineAssert(!isInitialized, "Render controller is already initialized!");

		what = 0;
		isInitialized = true;
	}

	void RenderController::Shutdown() {
		EngineAssert(isInitialized, "Render controller is not initialized!");

		//Take care of any remaining OpenGL jobs
		while(!glQueue.empty()){
			//Acquire next job
			GLJob& job = glQueue.front();

			//Run job
			job.func();

			//Mark job as done
			job.status->set_value();

			//Remove job from queue
			glQueue.pop();
		}

		isInitialized = false;
	}
}