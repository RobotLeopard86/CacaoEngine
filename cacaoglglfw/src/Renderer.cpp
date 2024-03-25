#include "Graphics/Rendering/RenderController.hpp"

#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "stb_image_write.h"

#include "Graphics/Window.hpp"
#include "GLRenderData.hpp"
#include "Events/EventSystem.hpp"

//For my sanity
#define nd ((GLRenderData*)nativeData)
#define thisnd ((GLRenderData*)this->nativeData)

namespace Cacao {
	void RenderController::ProcessFrame(Frame& frame){
		//Set up framebuffer for rendering
		/*GLuint colorBuffer, depthStencilRBO;
		glm::ivec2 winSize = Window::GetInstance()->GetSize();
		glGenRenderbuffers(1, &depthStencilRBO);
		glBindRenderbuffer(GL_RENDERBUFFER, depthStencilRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, winSize.x, winSize.y);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glGenTextures(1, &colorBuffer);
		glBindTexture(GL_TEXTURE_2D, colorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, winSize.x, winSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, nd->fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0); 
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencilRBO);
		EngineAssert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Failed to set up rendering framebuffer!");*/

		//Process mesh actions
		while(!nd->meshActionQueue.empty()) {
			//Acquire next mesh action
			MeshAction& ma = nd->meshActionQueue.front();

			//Process action
			if(ma.action == MeshAction::Action::Compile){
				ma.mesh->Compile();
			} else if(ma.action == MeshAction::Action::Release){
				ma.mesh->Release();
			} else {
				Logging::EngineLog("Cannot process mesh action with unknown action type!", LogLevel::Error);
			}

			//Remove action from queue
			nd->meshActionQueue.pop();
		}

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
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);

			//Draw the mesh
			obj.mesh->Draw();

			//Unbind shader
			obj.material.shader->Unbind();
		}

		//Draw skybox (if one exists)
		if(frame.skybox.has_value()) frame.skybox.value().Draw(frame.projection, frame.view);

		int width, height;
		glfwGetFramebufferSize((GLFWwindow*)Window::GetInstance()->GetNativeWindow(), &width, &height);
		GLsizei nrChannels = 3;
		GLsizei stride = nrChannels * width;
		stride += (stride % 4) ? (4 - stride % 4) : 0;
		GLsizei bufferSize = stride * height;
		std::vector<char> buffer(bufferSize);
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
		glReadBuffer(GL_BACK);
		glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
		stbi_flip_vertically_on_write(true);
		stbi_write_png("buf.out.png", width, height, nrChannels, buffer.data(), stride);

		/*//Un-setup framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteTextures(1, &colorBuffer);
		glDeleteRenderbuffers(1, &depthStencilRBO);

		float triv[] = {
			-1.0f, -1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			1.0f,  1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f
		}; 
		unsigned int trii[] = {
			0, 3, 2,
			0, 1, 2
		};

		unsigned int VBO, IBO, VAO;
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &IBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(triv), triv, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(trii), trii, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0); 
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &IBO);*/
	}

	void RenderController::Init() {
		EngineAssert(!initialized, "Cannot initialize initialized render controller!");

		//Take control of the OpenGL context
		glfwMakeContextCurrent((GLFWwindow*)Window::GetInstance()->GetNativeWindow());

		//Create native data and framebuffer
		nativeData = new GLRenderData();
		glGenFramebuffers(1, &(nd->fbo));

		EventManager::GetInstance()->SubscribeConsumer("MeshCompile", new EventConsumer([this](Event& e) {
			DataEvent<Mesh*>& de = static_cast<DataEvent<Mesh*>&>(e);
			thisnd->meshActionQueue.push({ .mesh = de.GetData(), .action = MeshAction::Action::Compile });
		}));
		EventManager::GetInstance()->SubscribeConsumer("MeshRelease", new EventConsumer([this](Event& e) {
			DataEvent<Mesh*>& de = static_cast<DataEvent<Mesh*>&>(e);
			thisnd->meshActionQueue.push({ .mesh = de.GetData(), .action = MeshAction::Action::Release });
		}));

		initialized = true;
	}

	void RenderController::Shutdown() {
		EngineAssert(initialized, "Cannot shutdown uninitialized render controller!");

		//Delete framebuffer and native data
		glDeleteFramebuffers(1, &(nd->fbo));
		delete nativeData;

		initialized = false;
	}
}