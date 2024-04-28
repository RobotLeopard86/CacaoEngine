#include "Graphics/Rendering/RenderController.hpp"

#include "glad/gl.h"
#include "GLFW/glfw3.h"

#include "Graphics/Window.hpp"
#include "Events/EventSystem.hpp"
#include "GLUtils.hpp"
#include "Core/Engine.hpp"
#include "Core/Exception.hpp"

namespace Cacao {
	//Queue of OpenGL jobs to process
	static std::queue<GLJob> glQueue;

	//Queue mutex
	static std::mutex queueMutex;

	void RenderController::UpdateGraphicsState() {
		//Process OpenGL jobs
		while(!glQueue.empty()) {
			//Acquire next job
			queueMutex.lock();
			GLJob& job = glQueue.front();
			queueMutex.unlock();

			//Run job
			//In case it throws an exception, send it back to the caller
			try {
				job.func();

				//Mark job as done
				job.status->set_value();
			} catch(...) {
				try {
					std::rethrow_exception(std::current_exception());
				} catch(const std::exception& e) {
					Logging::EngineLog(e.what(), LogLevel::Error);
				}
				job.status->set_exception(std::current_exception());
			}

			//Remove job from queue
			queueMutex.lock();
			glQueue.pop();
			queueMutex.unlock();
		}
	}

	void RenderController::ProcessFrame(Frame& frame) {
		//Send the frame into the GL queue
		std::shared_future<void> frameJob = InvokeGL([&frame]() {
			//Clear the screen
			glClearColor(0.765625f, 1.0f, 0.1015625f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//Render main scene
			for(RenderObject& obj : frame.objects) {
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
		});

		//Update the graphics state (will guarantee that the job is processed, so it doesn't need to be waited on)
		UpdateGraphicsState();
	}

	void EnqueueGLJob(GLJob& job) {
		queueMutex.lock();
		glQueue.push(job);
		queueMutex.unlock();
	}

	void RenderController::Init() {
		CheckException(!isInitialized, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Cannot initialize the initialized render controller!")
		isInitialized = true;
	}

	void RenderController::Shutdown() {
		CheckException(isInitialized, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Cannot shutdown the uinitialized render controller!")

		//Take care of any remaining OpenGL jobs
		while(!glQueue.empty()) {
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

	void Engine::RegisterBackendExceptions() {
		Exception::RegisterExceptionCode(100, "BadCompileState");
		Exception::RegisterExceptionCode(101, "BadBindState");
		Exception::RegisterExceptionCode(102, "GLFWError");
		Exception::RegisterExceptionCode(103, "OpenGLError");
		Exception::RegisterExceptionCode(104, "UniformUploadFailure");
		Exception::RegisterExceptionCode(105, "RenderThread");
	}
}