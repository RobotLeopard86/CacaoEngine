#include "3D/Skybox.hpp"

#include "Core/Log.hpp"
#include "Core/Engine.hpp"
#include "GLSkyboxData.hpp"
#include "GLUtils.hpp"

#include <future>

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
		nd->vaoReady = false;

		//Set texture
		texture = tex;
	}

	Skybox::Skybox(const Skybox& other) 
		: orientation(other.orientation), texture(other.texture) {
		//Copy native data
		nativeData = new GLSkyboxData();
		*nativeData = *(other.nativeData);
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
		skySamplerInfo.type = SpvType::SampledImage;
		spec.push_back(skySamplerInfo);

		//Create temporary data objects
		std::vector<uint32_t> v(vsCode, std::end(vsCode));
		std::vector<uint32_t> f(fsCode, std::end(fsCode));

		//Create and compile skybox shader object
		//Compile future is intentionally discarded as it will be done by the time this is used
		skyboxShader = new Shader(v, f, spec);
		skyboxShader->Compile();

		isSetup = true;
	}

	void Skybox::CommonCleanup(){
		//Temporary shader pointer for capturing
		Shader* shader = skyboxShader;
		GLJob job([shader]() {
			shader->Release();
			while(shader->IsCompiled()) {}
			delete shader;
		});
		EnqueueGLJob(job);

		isSetup = false;
	}

	void Skybox::Draw(glm::mat4 projectionMatrix, glm::mat4 viewMatrix){
		if(std::this_thread::get_id() != Engine::GetInstance()->GetThreadID()){
			Logging::EngineLog("Cannot draw skybox in non-rendering thread!", LogLevel::Error);
			return;
		}

		//Confirm that texture is compiled
		if(!texture->IsCompiled()){
			Logging::EngineLog("Skybox texture has not been compiled! Aborting draw.", LogLevel::Error);
		}

		//Set up OpenGL VAO if not already
		if(!nd->vaoReady) {
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

			nd->vaoReady = true;
		}

		//Create skybox matrices
		glm::mat4 skyView = glm::mat4(glm::mat3(viewMatrix));
		glm::mat4 skyTransform(1.0);
		skyTransform = glm::rotate(skyTransform, glm::radians(orientation.pitch), { 1.0, 0.0, 0.0 });
		skyTransform = glm::rotate(skyTransform, glm::radians(orientation.yaw), { 0.0, 1.0, 0.0 });
		skyTransform = glm::rotate(skyTransform, glm::radians(orientation.roll), { 0.0, 0.0, 1.0 });

		//Bind skybox shader and texture
		skyboxShader->Bind();

		//Upload data to shader
		skyboxShader->UploadCacaoData(projectionMatrix, skyView, skyTransform);
		ShaderUploadData sud;
		ShaderUploadItem skySampler;
		skySampler.data = std::any(texture);
		skySampler.target = "skybox";
		sud.push_back(skySampler);
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