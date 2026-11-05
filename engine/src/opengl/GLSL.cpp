#include "Targetgen.hpp"

#include "spirv_cross.hpp"
#include "spirv_parser.hpp"
#include "spirv_glsl.hpp"

#include <sstream>

namespace Cacao {
	GLSL GenerateGLSL(const libcacaoasset::Shader& in, bool es) {
		//Get SPIR-V
		std::vector<uint32_t> spv = GenerateSPV(in);

		//Parse the SPIR-V
		spirv_cross::Parser parser(std::move(spv));
		parser.parse();
		spirv_cross::ParsedIR& ir = parser.get_parsed_ir();

		//Set up SPIRV-Cross options
		spirv_cross::CompilerGLSL::Options opts;
		opts.version = es ? 300 : 410;
		opts.es = es;
		opts.enable_420pack_extension = false;
		opts.separate_shader_objects = true;

		//Make the compiler
		spirv_cross::CompilerGLSL glsl(std::move(ir));
		spirv_cross::ShaderResources res = glsl.get_shader_resources();
		glsl.set_common_options(opts);

		//Create output object
		GLSL code = {};
		auto [vsepName, fsepName] = GetEntrypointNames(in);

		//Get vertex shader code
		glsl.set_entry_point(vsepName, spv::ExecutionModel::ExecutionModelVertex);
		res = glsl.get_shader_resources();
		for(auto& out : res.stage_outputs) {
			std::stringstream newName;
			newName << "vsOut." << out.name.substr(27 + vsepName.size(), out.name.size());
			glsl.set_name(out.id, newName.str());
		}
		for(auto& img : res.sampled_images) {
			glsl.unset_decoration(img.id, spv::DecorationDescriptorSet);
		}
		code.vertex = glsl.compile();

		//Get fragment shader code
		glsl.set_entry_point(fsepName, spv::ExecutionModel::ExecutionModelFragment);
		res = glsl.get_shader_resources();
		for(auto& in : res.stage_inputs) {
			std::stringstream newName;
			newName << "vsOut." << in.name.substr(16, in.name.size());
			glsl.set_name(in.id, newName.str());
		}
		for(auto& img : res.sampled_images) {
			glsl.unset_decoration(img.id, spv::DecorationDescriptorSet);
		}
		code.fragment = glsl.compile();

		//Return the goodies
		return code;
	}
}