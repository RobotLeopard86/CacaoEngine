#include "compiler.hpp"
#include "toolutil.hpp"
#include "outform.hpp"

#include "libcacaocommon.hpp"
#include "libcacaoformats.hpp"

#include "spirv_cross.hpp"
#include "spirv_parser.hpp"
#include "spirv_glsl.hpp"

#include <sstream>
#include <fstream>

CacaoShaderCompiler::CacaoShaderCompiler() {
	//Initialize global session
	VLOG_NONL("Initializing global session... ");
	SlangResult r = slang::createGlobalSession(gSession.writeRef());
	CheckException(r == SLANG_OK && gSession, "Failed to create Slang global session!");
	VLOG("Done.");
}

std::pair<bool, std::string> CacaoShaderCompiler::compile(const std::filesystem::path& in, const std::filesystem::path& out) {
	CVLOG_SINGLE("Compiling " << in << ": ")

	//Create session
	CVLOG_NONL("\tCreating session... ");
	ComPtr<slang::ISession> session;
	{
		slang::SessionDesc sessionDesc = {};
		slang::TargetDesc tgtDesc = {};
		std::vector<slang::CompilerOptionEntry> entries;
		{
			slang::CompilerOptionEntry opt = {};
			opt.name = slang::CompilerOptionName::VulkanUseGLLayout;
			opt.value = slang::CompilerOptionValue {slang::CompilerOptionValueKind::Int, true, 0, nullptr, nullptr};
			entries.push_back(opt);
		}
		{
			slang::CompilerOptionEntry opt = {};
			opt.name = slang::CompilerOptionName::EmitSpirvDirectly;
			opt.value = slang::CompilerOptionValue {slang::CompilerOptionValueKind::Int, true, 0, nullptr, nullptr};
			entries.push_back(opt);
		}
		{
			slang::CompilerOptionEntry opt = {};
			opt.name = slang::CompilerOptionName::GenerateWholeProgram;
			opt.value = slang::CompilerOptionValue {slang::CompilerOptionValueKind::Int, true, 0, nullptr, nullptr};
			entries.push_back(opt);
		}
		{
			slang::CompilerOptionEntry opt = {};
			opt.name = slang::CompilerOptionName::MatrixLayoutColumn;
			opt.value = slang::CompilerOptionValue {slang::CompilerOptionValueKind::Int, true, 0, nullptr, nullptr};
			entries.push_back(opt);
		}
		tgtDesc.format = SLANG_SPIRV;
		tgtDesc.profile = gSession->findProfile("spirv_1_5");
		sessionDesc.targets = &tgtDesc;
		sessionDesc.targetCount = 1;
		sessionDesc.compilerOptionEntries = entries.data();
		sessionDesc.compilerOptionEntryCount = entries.size();
		SlangResult r = gSession->createSession(sessionDesc, session.writeRef());
		CompileCheck(r == SLANG_OK && session, "Failed to create Slang session!");
	}
	CVLOG("Done.");

	constexpr const char* cacaoModuleSrc =
#include "cacaoshaderbase.inc"
		;

	//Load Cacao Engine module
	CVLOG_NONL("\tLoading Cacao shader module... ");
	ComPtr<slang::IModule> cacaoModule;
	{
		ComPtr<slang::IBlob> diagnosticsBlob;
		cacaoModule = session->loadModuleFromSourceString("cacaoshaderbase", "cacaoshaderbase.slang", cacaoModuleSrc, diagnosticsBlob.writeRef());
		if(!cacaoModule) {
			std::stringstream err;
			err << "Failed to load Cacao shader module";
			if(diagnosticsBlob) {
				err << ":\n"
					<< (const char*)diagnosticsBlob->getBufferPointer();
			} else {
				err << "!";
			}
			CompileCheck(false, err.str());
		}
	}
	CVLOG("Done.");

	//Load source as module
	CVLOG_NONL("\tReading source file... ");
	std::ifstream srcStream(in);
	CompileCheck(srcStream.is_open(), "Failed to open source file!");
	std::string src(std::istreambuf_iterator<char>(srcStream), {});
	CVLOG("Done.");
	CVLOG_NONL("\tCreating source module... ");
	ComPtr<slang::IModule> mod;
	{
		ComPtr<slang::IBlob> diagnosticsBlob;
		mod = session->loadModuleFromSourceString(in.string().c_str(), nullptr, src.c_str(), diagnosticsBlob.writeRef());
		if(!mod) {
			std::stringstream err;
			err << "Failed to create source module";
			if(diagnosticsBlob) {
				err << ":\n"
					<< (const char*)diagnosticsBlob->getBufferPointer();
			} else {
				err << "!";
			}
			CompileCheck(false, err.str());
		}
	}
	CVLOG("Done.");

	//Validate and fetch entry points
	CVLOG_NONL("\tFetching entry points...");
	ComPtr<slang::IEntryPoint> vsep, fsep;
	CompileCheck(mod->findEntryPointByName("VS_main", vsep.writeRef()) == SLANG_OK,
		"Shader does not contain a vertex stage entry point or it is not named and marked correctly.\n"
		"The vertex stage entry point must be:\n"
		"\t- Marked with [shader(\"vertex\")]\n"
		"\t- Named VS_main");
	CompileCheck(mod->findEntryPointByName("FS_main", fsep.writeRef()) == SLANG_OK,
		"Shader does not contain a fragment stage entry point or it is not named and marked correctly.\n"
		"The fragment stage entry point must be:\n"
		"\t- Marked with [shader(\"fragment\")]\n"
		"\t- Named FS_main");
	CVLOG("Done.");

	//Ensure that the entrypoints are valid (aka they take in the proper types and output the proper types)
	slang::FunctionReflection* vsepFunc = vsep->getFunctionReflection();
	CompileCheck(vsepFunc->getParameterCount() == 2, "Shader's vertex stage entry point does not take 2 arguments!");
	slang::VariableReflection* vsp1 = vsepFunc->getParameterByIndex(0);
	ComPtr<slang::IBlob> vsp1TypeName;
	CompileCheck(vsp1->getType()->getFullName(vsp1TypeName.writeRef()) == SLANG_OK, "Failed to fetch vertex stage entry point parameter #1 type name for validation!");
	std::string vsp1Type((const char*)vsp1TypeName->getBufferPointer());
	CompileCheck(vsp1Type.compare("VSIn") == 0, "Shader's vertex stage entry point parameter #1 is not of the Cacao Engine vertex shader input type!");
	slang::TypeReflection* vsp2 = vsepFunc->getParameterByIndex(1)->getType();
	CompileCheck(vsp2->getKind() == slang::TypeReflection::Kind::Matrix && vsp2->getColumnCount() == 4 && vsp2->getRowCount() == 4 && vsp2->getScalarType() == slang::TypeReflection::ScalarType::Float32, "Shader's vertex stage entry point parameter #1 is not a float4x4!");
	slang::TypeReflection* fragOut = fsep->getFunctionReflection()->getReturnType();
	CompileCheck(fragOut->getKind() == slang::TypeReflection::Kind::Vector && fragOut->getScalarType() == slang::TypeReflection::ScalarType::Float32 && fragOut->getColumnCount() == 4 && fragOut->getRowCount() == 1, "Shader's fragment stage enrty point does not return a float4!");

	//Compose program
	CVLOG_NONL("\tComposing shader program... ");
	std::array<slang::IComponentType*, 4> componentTypes {
		cacaoModule,
		mod,
		vsep,
		fsep};
	ComPtr<slang::IComponentType> composed;
	{
		ComPtr<slang::IBlob> diagnosticsBlob;
		SlangResult r = session->createCompositeComponentType(componentTypes.data(), componentTypes.size(), composed.writeRef(), diagnosticsBlob.writeRef());
		if(r != SLANG_OK || !composed) {
			std::stringstream err;
			err << "Failed to compose shader program";
			if(diagnosticsBlob) {
				err << ":\n"
					<< (const char*)diagnosticsBlob->getBufferPointer();
			} else {
				err << "!";
			}
			CompileCheck(false, err.str());
		}
	}
	CVLOG("Done.");

	//Link program
	CVLOG_NONL("\tLinking shader program... ");
	ComPtr<slang::IComponentType> linked;
	{
		ComPtr<slang::IBlob> diagnosticsBlob;
		SlangResult r = composed->link(linked.writeRef(), diagnosticsBlob.writeRef());
		if(r != SLANG_OK || !linked) {
			std::stringstream err;
			err << "Failed to link shader program";
			if(diagnosticsBlob) {
				err << ":\n"
					<< (const char*)diagnosticsBlob->getBufferPointer();
			} else {
				err << "!";
			}
			CompileCheck(false, err.str());
		}
	}
	CVLOG("Done.");

	//Generate shader object for writing
	libcacaoformats::Shader shader;
	switch(format) {
		case OutputFormat::SPIRV: {
			shader.type = libcacaoformats::Shader::CodeType::SPIRV;
			auto [maybeCode, msg] = genSPV(linked);
			if(!maybeCode.has_value()) {
				std::stringstream err;
				err << "Failed to generate shader code";
				if(!msg.empty()) {
					err << ":\n"
						<< msg;
				} else {
					err << "!";
				}
				CompileCheck(false, err.str());
			}
			shader.code = maybeCode.value();
			break;
		}
		case OutputFormat::GLSL: {
			shader.type = libcacaoformats::Shader::CodeType::GLSL;
			auto [maybeCode, msg] = genGLSL(linked);
			if(!maybeCode.has_value()) {
				std::stringstream err;
				err << "Failed to generate shader code";
				if(!msg.empty()) {
					err << ":\n"
						<< msg;
				} else {
					err << "!";
				}
				CompileCheck(false, err.str());
			}
			shader.code = maybeCode.value();
			break;
		}
		default: break;
	}

	//Write shader
	CVLOG_NONL("\tWriting output file " << out << "... ");
	std::ofstream outStream(out, std::ios::binary);
	CompileCheck(outStream.is_open(), "Failed to open output file!");
	libcacaoformats::PackedEncoder encoder;
	encoder.EncodeShader(shader).ExportToStream(outStream);
	CVLOG("Done.");

	return {true, ""};
}

#define GenerateCheck(condition, ...)   \
	if(!(condition)) {                  \
		std::stringstream s;            \
		s << __VA_ARGS__;               \
		return {std::nullopt, s.str()}; \
	}

std::pair<std::optional<decltype(libcacaoformats::Shader::code)>, std::string> CacaoShaderCompiler::genSPV(ComPtr<slang::IComponentType> linked) {
	CVLOG_NONL("\tGenerating shader SPIR-V... ");
	ComPtr<slang::IBlob> spirv;
	{
		ComPtr<slang::IBlob> diagnosticsBlob;
		SlangResult r = linked->getTargetCode(0, spirv.writeRef(), diagnosticsBlob.writeRef());
		if(r != SLANG_OK || !spirv) {
			std::stringstream err;
			err << "Failed to generate shader SPIR-V";
			if(diagnosticsBlob) {
				err << ":\n"
					<< (const char*)diagnosticsBlob->getBufferPointer();
			} else {
				err << "!";
			}
			GenerateCheck(false, err.str());
		}
	}
	libcacaoformats::Shader::SPIRVCode code(spirv->getBufferSize() / 4);
	std::memcpy(code.data(), spirv->getBufferPointer(), spirv->getBufferSize());
	CVLOG("Done.");
	return {code, ""};
}

std::pair<std::optional<decltype(libcacaoformats::Shader::code)>, std::string> CacaoShaderCompiler::genGLSL(ComPtr<slang::IComponentType> linked) {
	//Run SPIR-V compile and get code
	auto [maybeCode, msg] = genSPV(linked);
	if(!maybeCode.has_value()) {
		std::stringstream err;
		err << "Failed to generate SPIR-V for GLSL conversion";
		if(!msg.empty()) {
			err << ":\n"
				<< msg;
		} else {
			err << "!";
		}
		GenerateCheck(false, err.str());
	}
	std::vector<uint32_t> spv;
	try {
		spv = std::get<std::vector<uint32_t>>(maybeCode.value());
	} catch(const std::exception& e) {
		GenerateCheck(false, "Generated SPIR-V was improperly created!");
	}

	//Parse the SPIR-V
	CVLOG_NONL("\tParsing SPIR-V IR... ");
	spirv_cross::Parser parser(std::move(spv));
	parser.parse();
	spirv_cross::ParsedIR& ir = parser.get_parsed_ir();
	CVLOG("Done.");

	//Set up SPIRV-Cross options
	CVLOG_NONL("\tConfiguring GLSL transpiler... ");
	spirv_cross::CompilerGLSL::Options opts;
	opts.version = 410;
	opts.es = false;
	opts.enable_420pack_extension = false;
	opts.separate_shader_objects = true;

	//Make the compiler
	spirv_cross::CompilerGLSL glsl(std::move(ir));
	spirv_cross::ShaderResources res = glsl.get_shader_resources();
	glsl.set_common_options(opts);
	CVLOG("Done.");

	//Remove image descriptor decorations
	CVLOG_NONL("\tPreprocessing decorations... ");
	for(auto& img : res.sampled_images) {
		glsl.unset_decoration(img.id, spv::DecorationDescriptorSet);
	}
	CVLOG("Done.");

	//Create output object
	libcacaoformats::Shader::GLSLCode code = {};

	//Get vertex shader code
	CVLOG_NONL("\tGenerating vertex stage GLSL... ");
	glsl.set_entry_point("VS_main", spv::ExecutionModel::ExecutionModelVertex);
	code.vertex = glsl.compile();
	CVLOG("Done.")

	//Get fragment shader code
	CVLOG_NONL("\tGenerating fragment stage GLSL... ");
	glsl.set_entry_point("FS_main", spv::ExecutionModel::ExecutionModelFragment);
	code.fragment = glsl.compile();
	CVLOG("Done.")

	//Return the goodies
	return {code, ""};
}