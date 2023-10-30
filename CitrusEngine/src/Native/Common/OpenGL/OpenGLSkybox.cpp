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

	Skybox* Skybox::Create(TextureCube tex){
		return new OpenGLSkybox(tex);
	}

	OpenGLSkybox::OpenGLSkybox(TextureCube tex){
		//Create pointer from cubemap
		texture = new TextureCube(tex);
		texture->Compile();
	}

	OpenGLSkybox::~OpenGLSkybox(){
		texture->Release();
		delete texture;
	}

	void Skybox::CommonSetup(){
		if(isSetup){
			Logging::EngineLog(LogLevel::Error, "Cannot set up skybox resources which are already set up!");
			return;
		}

		//Define skybox shader code

		std::string skyVert = R"(
			#version 330 core

			layout(location=0) in vec3 position;

			uniform mat4 projection;

			out vec3 texCoords;

			void main() {
				vec4 pos = projection * vec4(position, 0);
				gl_Position = pos.xyww;
				texCoords = position;
			}
		)";

		std::string skyFrag = R"(
			#version 330 core

			in vec3 texCoords;

			out vec4 color;

			uniform samplerCube tex;

			void main() {
				color = texture(tex, texCoords);
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
		//Save current GL culling mode and depth func
		GLint oldGLCull, oldGLDepthFunc;
		glGetIntegerv(GL_CULL_FACE_MODE, &oldGLCull);
		glGetIntegerv(GL_DEPTH_FUNC, &oldGLDepthFunc);

		//Bind skybox shader and texture
		skyboxShader->Bind();
		texture->Bind();

		//Set culling mode and depth func to be appropriate for skybox rendering
		glCullFace(GL_FRONT);
		glDepthFunc(GL_LEQUAL);

		//Create skybox projection
		glm::mat4 projection = StateManager::GetInstance()->GetActiveCamera()->GetViewProjectionMatrix() * glm::scale(glm::vec3(100.0f));

		//Upload projection to shader
		skyboxShader->UploadUniformMat4("projection", projection);

		//Render skybox mesh
		skyboxMesh->PureDraw();

		//Unbind shader and texture
		texture->Unbind();
		skyboxShader->Unbind();

		//Restore old culling mode and depth func
		glCullFace(oldGLCull);
		glDepthFunc(oldGLDepthFunc);
	}
}