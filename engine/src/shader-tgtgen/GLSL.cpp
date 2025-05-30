#include "Targetgen.hpp"
#include "CSO.hpp"

#include <sstream>

#include "spirv_cross.hpp"
#include "spirv_parser.hpp"
#include "spirv_glsl.hpp"

#ifndef BE_OPENGL
#error "Cannot compile GLSL target generator when OpenGL is disabled!"
#endif

namespace Targetgen {
	GLSL GenerateGLSL(ibytestream& in, bool es) {
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

		//Remove image descriptor decorations
		for(auto& img : res.sampled_images) {
			glsl.unset_decoration(img.id, spv::DecorationDescriptorSet);
		}

		//Create output object
		GLSL code = {};

		//Get vertex shader code
		glsl.set_entry_point("VS_main", spv::ExecutionModel::ExecutionModelVertex);
		code.vertex = glsl.compile();

		//Get fragment shader code
		glsl.set_entry_point("FS_main", spv::ExecutionModel::ExecutionModelFragment);
		code.fragment = glsl.compile();

		//Return the goodies
		return code;
	}
}