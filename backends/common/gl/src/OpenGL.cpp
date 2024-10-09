#include "Graphics/Rendering/RenderController.hpp"

#include "glad/gl.h"

#include "Graphics/Window.hpp"
#include "Events/EventSystem.hpp"
#include "GLUtils.hpp"
#include "Core/Engine.hpp"
#include "Core/Exception.hpp"
#include "ExceptionCodes.hpp"
#include "Graphics/Textures/Texture2D.hpp"
#include "UI/Shaders.hpp"
#include "Graphics/Textures/Cubemap.hpp"
#include "GLUIView.hpp"

constexpr glm::vec3 clearColorSRGB {float(0xCF) / 256, 1.0f, float(0x4D) / 256};

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

	void RenderController::ProcessFrame(std::shared_ptr<Frame> frame) {
		//Send the frame into the queue
		std::shared_future<void> frameTask = InvokeGL([frame]() {
			//Clear the screen
			//We use an obnoxious neon alligator green because it indicates that something is messed up if you can see it
			glm::vec3 clearColorLinear = glm::pow(clearColorSRGB, glm::vec3 {2.2f});
			glClearColor(clearColorLinear.r, clearColorLinear.g, clearColorLinear.b, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//Upload globals
			Shader::UploadCacaoGlobals(frame->projection, frame->view);

			//Render main scene
			for(RenderObject& obj : frame->objects) {
				//Upload material data to shader
				obj.material.shader->UploadData(obj.material.data);
				obj.material.shader->UploadCacaoLocals(obj.transformMatrix);

				//Bind shader
				obj.material.shader->Bind();

				//Configure OpenGL (ES)
				glEnable(GL_DEPTH_TEST);
				glDisable(GL_BLEND);
				glDepthFunc(GL_LESS);

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
						} else if(sui.data.type() == typeid(UIView*)) {
							UIView* view = std::any_cast<UIView*>(sui.data);
							view->Unbind();
						} else if(sui.data.type() == typeid(AssetHandle<Texture2D>)) {
							AssetHandle<Texture2D> tex = std::any_cast<AssetHandle<Texture2D>>(sui.data);
							tex->Unbind();
						} else if(sui.data.type() == typeid(AssetHandle<Cubemap>)) {
							AssetHandle<Cubemap> tex = std::any_cast<AssetHandle<Cubemap>>(sui.data);
							tex->Unbind();
						} else if(sui.data.type() == typeid(AssetHandle<UIView>)) {
							AssetHandle<UIView> view = std::any_cast<AssetHandle<UIView>>(sui.data);
							view->Unbind();
						}
					}
				}

				//Unbind shader
				obj.material.shader->Unbind();
			}

			//Draw skybox (if one exists)
			if(!frame->skybox.IsNull()) frame->skybox->Draw(frame->projection, frame->view);

			//Draw UI
			if(Engine::GetInstance()->GetGlobalUIView()->HasBeenRendered()) {
				//Create projection matrix
				glm::mat4 project = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f);

				//Upload uniforms
				ShaderUploadData uiud;
				uiud.emplace_back(ShaderUploadItem {.target = "uiTex", .data = std::any(Engine::GetInstance()->GetGlobalUIView().get())});
				uivsm->Bind();
				uivsm->UploadData(uiud);
				Shader::UploadCacaoGlobals(project, glm::identity<glm::mat4>());//Kinda scary but it'll get overwritten for the next frame

				//Configure OpenGL (ES)
				glDisable(GL_DEPTH_TEST);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				//Draw quad
				glBindVertexArray(uiVao);
				glDrawArrays(GL_TRIANGLES, 0, 6);
				glBindVertexArray(0);

				//Reset OpenGL (ES) configurations to normal
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_LESS);
				glDisable(GL_BLEND);

				//Unbind UI view texture and shader
				uivsm->Unbind();
				Engine::GetInstance()->GetGlobalUIView()->Unbind();
			}
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

		//Enable SRGB
		glEnable(GL_FRAMEBUFFER_SRGB_EXT);

		//Create globals UBO
		glGenBuffers(1, &globalsUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, globalsUBO);
		glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		//Compile UI view shader
		uivsm.Compile();

		//Set up UI view quad
		glGenVertexArrays(1, &uiVao);
		glGenBuffers(1, &uiVbo);
		glBindVertexArray(uiVao);
		glBindBuffer(GL_ARRAY_BUFFER, uiVbo);
		constexpr float quadData[18] = {
			0.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f};
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadData), quadData, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
		glBindVertexArray(uiVao);
		glBindVertexArray(0);

		//Compile UI element shaders
		GenShaders();
	}

	void RenderController::Shutdown() {
		CheckException(isInitialized, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Cannot shutdown the uninitialized render controller!")

		//Release UI element shaders
		DelShaders();

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

		//Delete global shader UBO
		glDeleteBuffers(1, &globalsUBO);

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
