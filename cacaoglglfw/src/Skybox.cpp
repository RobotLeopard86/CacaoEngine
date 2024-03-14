#include "3D/Skybox.hpp"

#include "Core/Log.hpp"
#include "GLSkyboxData.hpp"

#include "glad/gl.h"

#include "glm/gtx/transform.hpp"

//For my sanity
#define nd ((GLSkyboxData*)nativeData)

namespace Cacao {

	//Initialize static resources
	bool Skybox::isSetup = false;
	Shader* Skybox::skyboxShader = nullptr;

	Skybox::Skybox(Cubemap* tex) 
		: orientation({0, 0, 0}) {
		//Create native data
		nativeData = new GLSkyboxData();

		//Set texture
		texture = tex;

		//Set up OpenGL VAO

		//Generate vertex array and buffer
		glGenVertexArrays(1, &(nd->vao));
		glGenBuffers(1, &(nd->vbo));
		glBindVertexArray(nd->vao);
		glBindBuffer(GL_ARRAY_BUFFER, nd->vbo);

		//Load vertex data
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVerts), &skyboxVerts, GL_STATIC_DRAW);

		//Set up attributes
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

		//Bind VAO and unbind it again to save data
		glBindVertexArray(nd->vao);
		glBindVertexArray(0);
	}

	void Skybox::CommonSetup(){
		if(isSetup){
			Logging::EngineLog("Cannot set up skybox resources which are already set up!", LogLevel::Error);
			return;
		}

		//Define skybox shader specification
		ShaderSpec spec;
		ShaderItemInfo skySamplerInfo;
		skySamplerInfo.entryName = "skybox";
		skySamplerInfo.size = {1, 1};
		skySamplerInfo.type = spirv_cross::SPIRType::SampledImage;
		spec.push_back(skySamplerInfo);

		//Create temporary data objects
		std::vector<uint32_t> v(vsCode, std::end(vsCode));
		std::vector<uint32_t> f(fsCode, std::end(fsCode));

		//Create and compile skybox shader object
		skyboxShader = new Shader(v, f, spec);
		skyboxShader->Compile();

		isSetup = true;
	}

	void Skybox::CommonCleanup(){
		skyboxShader->Release();

		delete skyboxShader;

		isSetup = false;
	}

	void Skybox::Draw(glm::mat4 projectionMatrix, glm::mat4 viewMatrix){
		//Confirm that texture is compiled
		if(!texture->IsCompiled()){
			Logging::EngineLog("Skybox texture has not been compiled! Aborting draw.", LogLevel::Error);
		}

		//Create skybox matrices
		glm::mat4 skyView = glm::mat4(glm::mat3(viewMatrix));
		glm::mat4 skyTransform(1.0);
		skyTransform = glm::rotate(skyTransform, glm::radians(orientation.pitch), { 1.0, 0.0, 0.0 });
		skyTransform = glm::rotate(skyTransform, glm::radians(orientation.yaw), { 0.0, 1.0, 0.0 });
		skyTransform = glm::rotate(skyTransform, glm::radians(orientation.roll), { 0.0, 0.0, 1.0 });

		//Bind skybox shader and texture
		skyboxShader->Bind();
		texture->Bind(0);

		//Upload data to shader
		skyboxShader->UploadCacaoData(projectionMatrix, skyView, skyTransform);
		ShaderUploadData sud;
		ShaderUploadItem skySamplerID;
		skySamplerID.data = 0;
		skySamplerID.target = "skybox";
		sud.push_back(skySamplerID);
		skyboxShader->UploadData(sud);

		//Ensure skybox always drawn
		glDepthFunc(GL_LEQUAL);

		//Draw skybox
		glBindVertexArray(nd->vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		//Reset draw mode to make sure other objects draw correctly
		glDepthFunc(GL_LESS);

		//Unbind shader and texture
		texture->Unbind();
		skyboxShader->Unbind();
	}
}