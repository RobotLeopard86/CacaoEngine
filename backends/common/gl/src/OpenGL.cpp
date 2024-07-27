#include "Graphics/Rendering/RenderController.hpp"

#include "GLHeaders.hpp"

#include "Graphics/Window.hpp"
#include "Events/EventSystem.hpp"
#include "GLUtils.hpp"
#include "Core/Engine.hpp"
#include "Core/Exception.hpp"
#include "ExceptionCodes.hpp"
#include "Graphics/Textures/Texture2D.hpp"
#include "Graphics/Textures/Cubemap.hpp"
#include "GLUIView.hpp"

namespace Cacao {
	//Queue of OpenGL (ES) tasks to process
	static std::queue<Task> glQueue;

	//Queue mutex
	static std::mutex queueMutex;

	//UI quad assets
	static GLuint uiVao, uiVbo;
	static UIViewShaderManager uivsm;

	void RenderController::UpdateGraphicsState() {
		//Process OpenGL (ES) tasks
		while(!glQueue.empty()) {
			//Acquire next task
			queueMutex.lock();
			Task& task = glQueue.front();
			queueMutex.unlock();

			//Run task
			//In case it throws an exception, send it back to the caller
			try {
				task.func();

				//Mark task as done
				task.status->set_value();
			} catch(...) {
				task.status->set_exception(std::current_exception());
			}

			//Remove task from queue
			queueMutex.lock();
			glQueue.pop();
			queueMutex.unlock();
		}
	}

	void RenderController::ProcessFrame(Frame& frame) {
		//Send the frame into the queue
		std::shared_future<void> frameTask = InvokeGL([&frame]() {
			//Clear the screen
			//We use an obnoxious neon alligator green because it indicates that something is messed up if you can see it
			glClearColor(0.765625f, 1.0f, 0.1015625f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//Render main scene
			for(RenderObject& obj : frame.objects) {
				//Upload material data to shader
				obj.material.shader->UploadData(obj.material.data);
				obj.material.shader->UploadCacaoData(frame.projection, frame.view, obj.transformMatrix);

				//Bind shader
				obj.material.shader->Bind();

				//Configure OpenGL (ES) depth and blending behavior
				glEnable(GL_DEPTH_TEST);
				glEnable(GL_BLEND);
				glDepthFunc(GL_LESS);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				//Draw the mesh
				obj.mesh->Draw();

				//Unbind any textures
				const ShaderSpec& spec = obj.material.shader->GetSpec();
				for(ShaderUploadItem& sui : obj.material.data) {
					if(std::find_if(spec.begin(), spec.end(), [&sui](ShaderItemInfo sii) {
						   return (sii.type == SpvType::SampledImage && sii.entryName == sui.target);
					   }) != spec.end()) {
						if(sui.data.type() == typeid(Texture2D*)) {
							Texture2D* tex = std::any_cast<Texture2D*>(sui.data);
							tex->Unbind();
						} else if(sui.data.type() == typeid(Cubemap*)) {
							Cubemap* tex = std::any_cast<Cubemap*>(sui.data);
							tex->Unbind();
						}
					}
				}

				//Unbind shader
				obj.material.shader->Unbind();
			}

			//Draw skybox (if one exists)
			if(!frame.skybox.IsNull()) frame.skybox->Draw(frame.projection, frame.view);

			//Draw UI
			Engine::GetInstance()->GetGlobalUIView()->Bind(7);//Why seven? Because I say so.
			ShaderUploadData uiud;
			uiud.emplace_back({.target = "uiTex", .data = std::make_any(7)});
			uivsm->UploadData(uiud);
			uivsm->Bind();
			glDisable(GL_DEPTH_TEST);
			glBindVertexArray(uiVao);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);
			glEnable(GL_DEPTH_TEST);
			Engine::GetInstance()->GetGlobalUIView()->Unbind();
		});

		//Update the graphics state (will guarantee that the task is processed, so it doesn't need to be waited on)
		UpdateGraphicsState();
	}

	void EnqueueGLJob(Task& task) {
		queueMutex.lock();
		glQueue.push(task);
		queueMutex.unlock();
	}

	void RenderController::Init() {
		CheckException(!isInitialized, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Cannot initialize the initialized render controller!")
		isInitialized = true;

		uivsm = {};

		//Compile UI view shader
		uivsm.Compile();

		//Set up UI view quad
		glGenVertexArrays(1, &uiVao);
		glGenBuffers(1, &uiVbo);
		glBindVertexArray(uiVao);
		glBindBuffer(GL_ARRAY_BUFFER, uiVbo);
		constexpr float quadData[] = {
			0.0f, 1.0f, 0.0f 1.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f};
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadData), quadData, GL_STATIC_DRAW);
		glBindVertexArray(uiVao);
		glBindVertexArray(0);
	}

	void RenderController::Shutdown() {
		CheckException(isInitialized, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Cannot shutdown the uninitialized render controller!")

		//Clean up UI view quad
		glDeleteBuffers(1, &uiVbo);
		glDeleteVertexArrays(1, &uiVao);

		//Release UI view shader
		uivsm.Release();

		//Take care of any remaining OpenGL (ES) tasks
		while(!glQueue.empty()) {
			//Acquire next task
			Task& task = glQueue.front();

			//Run task
			task.func();

			//Mark task as done
			task.status->set_value();

			//Remove task from queue
			glQueue.pop();
		}

		isInitialized = false;
	}

	void RegisterGraphicsExceptions() {
		Exception::RegisterExceptionCode(100, "BadCompileState");
		Exception::RegisterExceptionCode(101, "BadBindState");
		Exception::RegisterExceptionCode(102, "GLError");
		Exception::RegisterExceptionCode(103, "UniformUploadFailure");
		Exception::RegisterExceptionCode(104, "RenderThread");
		Exception::RegisterExceptionCode(105, "UnsupportedType");
	}
}
