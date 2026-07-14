//============== DEVELOPER WARNING ==============
//
// Due to the inclusion of generated built-in
// asset files in this file, clangd is known
// to be VERY slow while working here.
//
//===============================================

#include "Cacao/ResourceManager.hpp"
#include "impl/Mesh.hpp"
#include "impl/Shader.hpp"
#include "impl/ResourceManager.hpp"
#include "ImplAccessor.hpp"

#include <cstdint>
#include <array>
#include <optional>

#include "libcacaoasset.hpp"
#include "Bytestream.hpp"

namespace Cacao {
	namespace assets {
#include "capsule.inc"
#include "cone.inc"
#include "cube.inc"
#include "cylinder.inc"
#include "quad.inc"
#include "quadpyramid.inc"
#include "sphere.inc"
#include "tripyramid.inc"
#include "skyshader.inc"
	}

	template<std::size_t S>
	std::shared_ptr<Mesh> _GenMesh(const std::array<unsigned char, S>& bin, const std::string& meshName, const std::string& addr) {
		//Create model and get mesh from it
		std::vector<unsigned char> modelBin(bin.begin(), bin.end());
		std::shared_ptr<Model> mdl = Model::Create(std::move(modelBin), std::format("a:internal_tmpmdl_{}", meshName));
		std::shared_ptr<Mesh> badMesh = mdl->GetMesh(meshName);

		//Recreate mesh using bad mesh data to ensure proper address
		std::shared_ptr<Mesh> goodMesh = Mesh::Create(std::move(IMPL(Mesh, *badMesh).vertices), std::move(IMPL(Mesh, *badMesh).indices), addr);
		badMesh.reset();
		goodMesh->Bake();
		return goodMesh;
	}

	template<std::size_t S>
	std::shared_ptr<Shader> _GenShader(const std::array<unsigned char, S>& bin, const std::string& addr, std::optional<Shader::Impl::CustomCompileSettings> settings = {}) {
		//Decode shader binary
		std::vector<unsigned char> binVec(bin.begin(), bin.end());
		ibytestream* ibs = new ibytestream(binVec);
		libcacaoasset::Shader shader;
		try {
			shader = libcacaoasset::DecodeShader(ibs);
		} catch(const std::exception& e) {
			Check<ExternalException>(false, e.what());
			throw std::runtime_error("UNREACHABLE CODE!!! HOW DID YOU GET HERE?!");//This will never be reached because of the Check call, but the compiler doesn't know what Check does, so we have to spell it out like it's 3
		}

		//Create shader object
		std::shared_ptr<Shader> s = Shader::Create(std::move(shader.irCode), shader.descriptor, addr);
		IMPL(Shader, *s).customSettings = settings;
		s->Bake();
		return s;
	}

	void ResourceManager::SetupBuiltins() {
		//Meshes
		std::vector<exathread::Future<std::shared_ptr<Mesh>>> mfuts;
		mfuts.push_back(Engine::Get().GetThreadPool()->submit([]() { return _GenMesh(assets::capsule, "DefaultCapsule", "a:builtin_capsule"); }));
		mfuts.push_back(Engine::Get().GetThreadPool()->submit([]() { return _GenMesh(assets::cone, "DefaultCone", "a:builtin_cone"); }));
		mfuts.push_back(Engine::Get().GetThreadPool()->submit([]() { return _GenMesh(assets::cube, "DefaultCube", "a:builtin_cube"); }));
		mfuts.push_back(Engine::Get().GetThreadPool()->submit([]() { return _GenMesh(assets::cylinder, "DefaultCylinder", "a:builtin_cylinder"); }));
		mfuts.push_back(Engine::Get().GetThreadPool()->submit([]() { return _GenMesh(assets::quad, "DefaultQuad", "a:builtin_quad"); }));
		mfuts.push_back(Engine::Get().GetThreadPool()->submit([]() { return _GenMesh(assets::quadpyramid, "DefaultQuadPyramid", "a:builtin_quadpyramid"); }));
		mfuts.push_back(Engine::Get().GetThreadPool()->submit([]() { return _GenMesh(assets::sphere, "DefaultSphere", "a:builtin_sphere"); }));
		mfuts.push_back(Engine::Get().GetThreadPool()->submit([]() { return _GenMesh(assets::tripyramid, "DefaultTriPyramid", "a:builtin_tripyramid"); }));
		exathread::MultiFuture<std::shared_ptr<Mesh>> meshes(std::move(mfuts));

		//Shaders
		std::vector<exathread::Future<std::shared_ptr<Shader>>> sfuts;
		sfuts.push_back(Engine::Get().GetThreadPool()->submit([]() {
			Shader::Impl::CustomCompileSettings cs = {};
			cs.blendUseSrc = false;
			cs.depth = Shader::Impl::CustomCompileSettings::Depth::Lequal;
			return _GenShader(assets::skyshader, "a:internal_skyshader", cs);
		}));
		exathread::MultiFuture<std::shared_ptr<Shader>> shaders(std::move(sfuts));

		//Assign meshes
		meshes.await();
		impl->builtinMeshes["a:builtin_capsule"] = meshes.results()[0];
		impl->builtinMeshes["a:builtin_cone"] = meshes.results()[1];
		impl->builtinMeshes["a:builtin_cube"] = meshes.results()[2];
		impl->builtinMeshes["a:builtin_cylinder"] = meshes.results()[3];
		impl->builtinMeshes["a:builtin_quad"] = meshes.results()[4];
		impl->builtinMeshes["a:builtin_quadpyramid"] = meshes.results()[5];
		impl->builtinMeshes["a:builtin_sphere"] = meshes.results()[6];
		impl->builtinMeshes["a:builtin_tripyramid"] = meshes.results()[7];

		//Assign shaders
		shaders.await();
		impl->builtinShaders["a:internal_skyshader"] = shaders.results()[0];

		//Complete
		impl->builtinsReady = true;
	}

	void ResourceManager::CleanupBuiltins() {
		impl->builtinsReady = false;
		for(auto& [_, shader] : impl->builtinShaders) {
			shader->Discard();
			shader.reset();
		}
		impl->builtinShaders.clear();
		for(auto& [_, mesh] : impl->builtinMeshes) {
			mesh->Discard();
			mesh.reset();
		}
		impl->builtinMeshes.clear();
	}

	std::shared_ptr<Mesh> ResourceManager::LoadBuiltinMesh(const std::string& id) {
		//Get and bake built-in assets if they haven't been yet
		if(!impl->builtinsReady) SetupBuiltins();

		//Return the mesh the user wants
		return impl->builtinMeshes[id];
	}

	std::shared_ptr<Shader> ResourceManager::LoadBuiltinShader(const std::string& id) {
		//Get and bake built-in assets if they haven't been yet
		if(!impl->builtinsReady) SetupBuiltins();

		//Return the shader the user wants
		return impl->builtinShaders[id];
	}
}