#include "Native/Common/OpenGL/OpenGLSkybox.hpp"

#include "Core/Log.hpp"

#include "Utilities/StateManager.hpp"

#include "glad/gl.h"

#include "glm/gtx/transform.hpp"

namespace CitrusEngine {

	//Initialize static resources
	bool Skybox::isSetup = false;
	Shader* Skybox::skyboxShader = nullptr;

	Skybox* Skybox::Create(Texture2D* tex){
		return new OpenGLSkybox(tex, {
			{ 0.375, 0 },
			{ 0.625, 0 },
			{ 0.625, 0.25 },
			{ 0.375, 0.25 },
			{ 0.375, 0.25 },
			{ 0.625, 0.25 },
			{ 0.625, 0.5 },
			{ 0.375, 0.5 },
			{ 0.375, 0.5 },
			{ 0.625, 0.5 },
			{ 0.625, 0.75 },
			{ 0.375, 0.75 },
			{ 0.375, 0.75 },
			{ 0.625, 0.75 },
			{ 0.625, 1 },
			{ 0.375, 1 },
			{ 0.125, 0.5 },
			{ 0.375, 0.5 },
			{ 0.375, 0.75 },
			{ 0.125, 0.75 },
			{ 0.625, 0.5 },
			{ 0.875, 0.5 },
			{ 0.875, 0.75 },
			{ 0.625, 0.75 }
		});
	}

	Skybox* Skybox::CreateWithCustomTexCoords(Texture2D* tex, std::vector<glm::vec2> texCoords){
		return new OpenGLSkybox(tex, texCoords);
	}

	OpenGLSkybox::OpenGLSkybox(Texture2D* tex, std::vector<glm::vec2> tcs){
		texture = tex;
		texCoords = tcs;

		//Build mesh
		//Define vertex and index data storage for mesh
		std::vector<Vertex> verts;
		std::vector<glm::uvec3> inds;

		//Add vertices (with positions and texture coordinates, other info irrelevant to skyboxes)
		verts.push_back({ { -1, -1, 1 }, tcs[0] });
		verts.push_back({ { -1, 1, 1 }, tcs[1] });
		verts.push_back({ { -1, 1, -1 }, tcs[2] });
		verts.push_back({ { -1, -1, -1 }, tcs[3] });
		verts.push_back({ { -1, -1, -1 }, tcs[4] });
		verts.push_back({ { -1, 1, -1 }, tcs[5] });
		verts.push_back({ { 1, 1, -1 }, tcs[6] });
		verts.push_back({ { 1, -1, -1 }, tcs[7] });
		verts.push_back({ { 1, -1, -1 }, tcs[8] });
		verts.push_back({ { 1, 1, -1 }, tcs[9] });
		verts.push_back({ { 1, 1, 1 }, tcs[10] });
		verts.push_back({ { 1, -1, 1 }, tcs[11] });
		verts.push_back({ { 1, -1, 1 }, tcs[12] });
		verts.push_back({ { 1, 1, 1 }, tcs[13] });
		verts.push_back({ { -1, 1, 1 }, tcs[14] });
		verts.push_back({ { -1, -1, 1 }, tcs[15] });
		verts.push_back({ { -1, -1, -1 }, tcs[16] });
		verts.push_back({ { 1, -1, -1 }, tcs[17] });
		verts.push_back({ { 1, -1, 1 }, tcs[18] });
		verts.push_back({ { -1, -1, 1 }, tcs[19] });
		verts.push_back({ { 1, 1, -1 }, tcs[20] });
		verts.push_back({ { -1, 1, -1 }, tcs[21] });
		verts.push_back({ { -1, 1, 1 }, tcs[22] });
		verts.push_back({ { 1, 1, 1 }, tcs[23] });

		//Add indices
		inds.push_back({ 0, 1, 2 });
		inds.push_back({ 0, 2, 3 });
		inds.push_back({ 4, 5, 6 });
		inds.push_back({ 4, 6, 7 });
		inds.push_back({ 8, 9, 10 });
		inds.push_back({ 8, 10, 11 });
		inds.push_back({ 12, 13, 14 });
		inds.push_back({ 12, 14, 15 });
		inds.push_back({ 16, 17, 18 });
		inds.push_back({ 16, 18, 19 });
		inds.push_back({ 20, 21, 22 });
		inds.push_back({ 20, 22, 23 });

		//Create and compile mesh object
		mesh = Mesh::Create(verts, inds);
		mesh->Compile();
	}

	OpenGLSkybox::~OpenGLSkybox() {
		if(mesh->IsCompiled()) mesh->Release();
		delete mesh;
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
			layout(location=1) in vec2 tc;
			out vec2 texCoords;

			uniform mat4 projection;
			uniform mat4 view;

			void main()
			{
				texCoords = tc;
				vec4 skypos = projection * view * vec4(pos, 1.0);
				gl_Position = skypos.xyww;
			}
		)";

		std::string skyFrag = R"(
			#version 330 core

			out vec4 color;
			in vec2 texCoords;

			uniform sampler2D skybox;

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

		//Upload projection and size to shader
		skyboxShader->UploadUniformMat4("projection", StateManager::GetInstance()->GetActiveCamera()->GetProjectionMatrix());
		skyboxShader->UploadUniformMat4("view", skyView);

		//Ensure skybox always drawn
		glDepthFunc(GL_LEQUAL);

		//Render skybox mesh
		mesh->PureDraw();

		//Reset draw mode to make sure other objects draw correctly
		glDepthFunc(GL_LESS);

		//Unbind shader and texture
		texture->Unbind();
		skyboxShader->Unbind();
	}
}