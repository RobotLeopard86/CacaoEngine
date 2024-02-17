#pragma once

#include <vector>
#include <filesystem>
#include <map>
#include <any>

#include "Core/Log.hpp"
#include "Core/Assert.hpp"
#include "Utilities/MiscUtils.hpp"

#include "spirv_cross.hpp"
#include "spirv_reflect.h"
#include "glm/glm.hpp"

namespace Cacao {
	//Shader data system

	//Shader data entry
	struct ShaderDataEntry {
		//Actual data
		std::any data;

		//Data for rendering system
		//If type of data changes, these values MUST be updated

		//Base type (e.g. int, float)
		spirv_cross::TypeID type;

		//Data size (x for number of vector components, y for number of vectors in a matrix)
		//Example: a vec3 would be {3, 1}, a mat4 would be {4, 4}, and a scalar would be {1, 1}
		glm::ivec2 size;
	};

	struct ShaderData {
	public:
		ShaderDataEntry& operator[](std::string item){
			if(!data.contains(item)) {
				data.insert_or_assign(item, ShaderDataEntry{});
			}
			return data.at(item);
		}

		bool EntryExists(std::string item){
			return data.contains(item);
		}

		void RemoveEntry(std::string entry){
			if(!data.contains(entry)){
				Logging::EngineLog("Can't remove nonexistent shader data entry!", LogLevel::Error);
				return;
			}
			data.erase(entry);
		}
	private:
		std::map<std::string, ShaderDataEntry> data;
	};

    //Must be implemented per-rendering API
	//Shader class
    class Shader {
    public:
		//Create a shader from raw SPIR-V code loaded separately
		Shader(std::vector<uint32_t> vertex, std::vector<uint32_t> fragment);
		//Create a shader from file paths
		Shader(std::filesystem::path vertex, std::filesystem::path fragment);

		~Shader() {
			if(compiled && bound) Unbind();
			if(compiled) Release();
			delete nativeData;
		}

        //Use this shader
        void Bind();
        //Don't use this shader
        void Unbind();
        //Compile shader to be used later
        void Compile();
        //Delete shader when no longer needed
        void Release();

        //Is shader compiled?
        bool IsCompiled() { return compiled; }

        //Is shader bound?
        bool IsBound() { return bound; }

		//Read-only access to native data
		const NativeData* GetNativeData() { return nativeData; }

		//Upload uniform data
		void UploadUniformData(ShaderData& data);

		//If this variable is true, the shader will be expected to take in lighting data
		bool lit;
    private:
        bool compiled;
        bool bound;
		NativeData* nativeData;

		SpvReflectTypeDescription vertexDataDesc, fragmentDataDesc;
    };
}