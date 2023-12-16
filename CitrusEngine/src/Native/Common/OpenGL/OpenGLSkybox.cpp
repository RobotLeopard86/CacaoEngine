#include "Native/Common/OpenGL/OpenGLSkybox.hpp"

#include "Core/Log.hpp"

#include "Utilities/StateManager.hpp"

#include "glad/gl.h"

#include "glm/gtx/transform.hpp"

namespace CitrusEngine {

	//Initialize static resources
	bool Skybox::isSetup = false;
	Mesh* Skybox::skyboxMesh = nullptr;
	Shader* Skybox::skyboxShader = nullptr;

	Skybox* Skybox::Create(TextureCube* tex){
		return new OpenGLSkybox(tex);
	}

	OpenGLSkybox::OpenGLSkybox(TextureCube* tex){
		//Create pointer from cubemap
		texture = tex;
	}

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

			mat4 scale(float scaleFactor) {
				return mat4(
					vec4(scaleFactor, 0.0, 0.0, 0.0),
					vec4(0.0, scaleFactor, 0.0, 0.0),
					vec4(0.0, 0.0, scaleFactor, 0.0),
					vec4(0.0, 0.0, 0.0, 1.0)
				);
			}

			void main()
			{
				texCoords = normalize(pos);
				vec4 skypos = scale(1000) * projection * vec4(pos, 1.0);
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

		//Define vertex and index data storage for mesh
		std::vector<Vertex> verts;
		std::vector<glm::uvec3> inds;

		//Add vertices (only position specified because tex coords are done automatically and other data is irrelevant to skyboxes)
		verts.push_back({ { -1, -1, 1 } });
		verts.push_back({ { 1, -1, 1 } });
		verts.push_back({ { -1, 1, 1 } });
		verts.push_back({ { 1, 1, 1 } });
		verts.push_back({ { -1, -1, -1 } });
		verts.push_back({ { 1, -1, -1 } });
		verts.push_back({ { -1, 1, -1 } });
		verts.push_back({ { 1, 1, -1 } });

		//Add indices
		inds.push_back({ 2, 6, 7 });
		inds.push_back({ 2, 3, 7 });
		inds.push_back({ 0, 4, 5 });
		inds.push_back({ 0, 1, 5 });
		inds.push_back({ 0, 2, 6 });
		inds.push_back({ 0, 4, 6 });
		inds.push_back({ 1, 3, 7 });
		inds.push_back({ 1, 5, 7 });
		inds.push_back({ 0, 2, 3 });
		inds.push_back({ 0, 1, 3 });
		inds.push_back({ 4, 6, 7 });
		inds.push_back({ 4, 5, 7 });

		//Create and compile mesh object
		skyboxMesh = Mesh::Create(verts, inds);
		skyboxMesh->Compile();

		isSetup = true;
	}

	void Skybox::CommonCleanup(){
		skyboxShader->Release();
		skyboxMesh->Release();

		delete skyboxShader;
		delete skyboxMesh;

		isSetup = false;
	}

	void OpenGLSkybox::Draw(){
		//Confirm that texture is compiled
		if(!texture->IsCompiled()){
			Logging::EngineLog(LogLevel::Warn, "Skybox texture compilation required prior to draw, compiling now...");
			texture->Compile();
		}

		//Bind skybox shader and texture
		skyboxShader->Bind();
		texture->Bind();

		//Create skybox projection
		glm::mat4 projection = glm::mat4(glm::mat3(StateManager::GetInstance()->GetActiveCamera()->GetViewMatrix())) * StateManager::GetInstance()->GetActiveCamera()->GetProjectionMatrix();

		//Upload projection and size to shader
		skyboxShader->UploadUniformMat4("projection", projection);

		//Ensure skybox always drawn
		glDepthFunc(GL_LEQUAL);

		//Render skybox mesh
		skyboxMesh->PureDraw();

		//Reset draw mode to make sure other objects draw correctly
		glDepthFunc(GL_LESS);

		//Unbind shader and texture
		texture->Unbind();
		skyboxShader->Unbind();
	}
}