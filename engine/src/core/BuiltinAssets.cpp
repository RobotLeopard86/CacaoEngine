#include "Cacao/ResourceManager.hpp"
#include "impl/Mesh.hpp"
#include "impl/ResourceManager.hpp"
#include "ImplAccessor.hpp"

#include <cstdint>
#include <array>

//============== DEVELOPER WARNING ==============
//
// Due to the inclusion of generated built-in
// asset files in this file, clangd is known
// to be VERY slow while working here.
//
//===============================================

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
	}

	template<std::size_t S>
	std::shared_ptr<Mesh> _GenMesh(const std::array<unsigned char, S>& bin, const std::string& meshName, const std::string& addr) {
		//Create model and get mesh from it
		std::vector<unsigned char> modelBin(bin.begin(), bin.end());
		std::shared_ptr<Model> mdl = Model::Create(std::move(modelBin), "m:TEMP");
		std::shared_ptr<Mesh> badMesh = mdl->GetMesh(meshName);

		//Recreate mesh using bad mesh data to ensure proper address
		std::shared_ptr<Mesh> goodMesh = Mesh::Create(std::move(IMPL(Mesh, *badMesh).vertices), std::move(IMPL(Mesh, *badMesh).indices), addr);
		badMesh.reset();
		return goodMesh;
	}

	std::shared_ptr<Mesh> ResourceManager::LoadBuiltinMesh(const std::string& id) {
		//Get and bake built-in meshes if they haven't been yet
		if(impl->builtinMeshes.empty()) {
			impl->builtinMeshes["a:builtin_capsule"] = _GenMesh(assets::capsule, "DefaultCapsule", "a:builtin_capsule");
			impl->builtinMeshes["a:builtin_cone"] = _GenMesh(assets::capsule, "DefaultCone", "a:builtin_cone");
			impl->builtinMeshes["a:builtin_cube"] = _GenMesh(assets::cube, "DefaultCube", "a:builtin_cube");
			impl->builtinMeshes["a:builtin_cylinder"] = _GenMesh(assets::cylinder, "DefaultCylinder", "a:builtin_cylinder");
			impl->builtinMeshes["a:builtin_quad"] = _GenMesh(assets::quad, "DefaultQuad", "a:builtin_quad");
			impl->builtinMeshes["a:builtin_quadpyramid"] = _GenMesh(assets::quadpyramid, "DefaultQuadPyramid", "a:builtin_quadpyramid");
			impl->builtinMeshes["a:builtin_sphere"] = _GenMesh(assets::sphere, "DefaultSphere", "a:builtin_sphere");
			impl->builtinMeshes["a:builtin_tripyramid"] = _GenMesh(assets::tripyramid, "DefaultTriPyramid", "a:builtin_tripyramid");
		}

		//Return the mesh the user wants
		return impl->builtinMeshes[id];
	}
}