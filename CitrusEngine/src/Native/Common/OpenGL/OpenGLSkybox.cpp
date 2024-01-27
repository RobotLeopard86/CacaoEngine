#include "Native/Common/OpenGL/OpenGLSkybox.hpp"

#include "Core/Log.hpp"

#include "Utilities/StateManager.hpp"

#include "glad/gl.h"

#include "glm/gtx/transform.hpp"

namespace CitrusEngine {

	//Initialize static resources
	bool Skybox::isSetup = false;
	Shader* Skybox::skyboxShader = nullptr;
	float OpenGLSkybox::skyboxVerts[] = {
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};

	Skybox* Skybox::Create(Cubemap* tex){
		return new OpenGLSkybox(tex);
	}

	OpenGLSkybox::OpenGLSkybox(Cubemap* tex){
		texture = tex;

		//Set up OpenGL VAO

		//Generate vertex array and buffer
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		//Load vertex data
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVerts), &skyboxVerts, GL_STATIC_DRAW);

		//Set up attributes
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

		//Bind VAO and unbind it again to save data
		glBindVertexArray(vao);
		glBindVertexArray(0);
	}

	OpenGLSkybox::~OpenGLSkybox() {}

	void Skybox::CommonSetup(){
		if(isSetup){
			Logging::EngineLog(LogLevel::Error, "Cannot set up skybox resources which are already set up!");
			return;
		}

		//Define skybox shader code

		std::string skyVert = R"(
			#version 330 core

			layout(location=0) in vec3 pos;

			out vec3 texCoords;

			uniform mat4 projection;
			uniform mat4 view;

			void main()
			{
				texCoords = pos;
				vec4 skypos = projection * view * vec4(pos, 1.0);
				gl_Position = skypos.xyww;
			}
		)";

		std::string skyFrag = R"(
			#version 330 core

			out vec4 color;
			in vec3 texCoords;

			uniform samplerCube skybox;

			void main()
			{
				color = texture(skybox, texCoords);
			}
		)";

		//Create and compile skybox shader object
		skyboxShader = Shader::Create(skyVert, skyFrag);
		skyboxShader->Compile();

		isSetup = true;
	}

	void Skybox::CommonCleanup(){
		skyboxShader->Release();

		delete skyboxShader;

		isSetup = false;
	}

	void OpenGLSkybox::Draw(){
		//Confirm that texture is compiled
		if(!texture->IsCompiled()){
			Logging::EngineLog(LogLevel::Warn, "Skybox texture compilation required prior to draw, compiling now...");
			texture->Compile();
		}

		//Bind skybox shader  and texture
		skyboxShader->Bind();
		texture->Bind();

		//Create skybox version of view matrix
		glm::mat4 skyView = glm::mat4(glm::mat3(StateManager::GetInstance()->GetActiveCamera()->GetViewMatrix()));

		//Upload projection and view matrices to shader
		skyboxShader->UploadUniformMat4("projection", StateManager::GetInstance()->GetActiveCamera()->GetProjectionMatrix());
		skyboxShader->UploadUniformMat4("view", skyView);

		//Ensure skybox always drawn
		glDepthFunc(GL_LEQUAL);

		//Draw skybox
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		//Reset draw mode to make sure other objects draw correctly
		glDepthFunc(GL_LESS);

		//Unbind shader and texture
		texture->Unbind();
		skyboxShader->Unbind();
	}
}