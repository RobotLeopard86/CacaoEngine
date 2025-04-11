#include "compiler.hpp"
#include "logutil.hpp"
#include "outform.hpp"

#include "libcacaocommon.hpp"
#include "libcacaoformats.hpp"

#include <sstream>
#include <fstream>

CacaoShaderCompiler::CacaoShaderCompiler() {
	//Initialize global session
	VLOG_NONL("Initializing global session... ");
	SlangResult r = slang::createGlobalSession(gSession.writeRef());
	CheckException(r == SLANG_OK && gSession, "Failed to create Slang global session!");
	VLOG("Done.");
}

#define CompileCheck(condition, ...) \
	if(!(condition)) {               \
		std::stringstream s;         \
		s << __VA_ARGS__;            \
		return {false, s.str()};     \
	}

static std::stringstream lastMsg;

#define CVLOG_NONL(...)                    \
	VLOG_NONL("\x1b[2K\r" << __VA_ARGS__); \
	lastMsg.str("");                       \
	lastMsg << __VA_ARGS__;
#define CVLOG(...) VLOG("\x1b[2K\r" << lastMsg.str() << __VA_ARGS__)
#define CVLOG_SINGLE(...) VLOG("\x1b[2K\r" << __VA_ARGS__)

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
			opt.value = slang::CompilerOptionValue {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr};
			entries.push_back(opt);
		}
		switch(format) {
			case OutputFormat::SPIRV:
				tgtDesc.format = SLANG_SPIRV;
				tgtDesc.profile = gSession->findProfile("spirv_1_5");
				{
					slang::CompilerOptionEntry opt = {};
					opt.name = slang::CompilerOptionName::EmitSpirvDirectly;
					opt.value = slang::CompilerOptionValue {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr};
					entries.push_back(opt);
				}
				break;
			case OutputFormat::GLSL:
				tgtDesc.format = SLANG_GLSL;
				tgtDesc.profile = gSession->findProfile("glsl_410");
				break;
			default: break;
		}
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
		mod = session->loadModuleFromSourceString(in.c_str(), nullptr, src.c_str(), diagnosticsBlob.writeRef());
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
		"Shader does not contain a vertex entry point or it is not named and marked correctly.\n"
		"The vertex entry point must be:\n"
		"\t- Marked with [shader(\"vertex\")]\n"
		"\t- Named VS_main");
	CompileCheck(mod->findEntryPointByName("FS_main", fsep.writeRef()) == SLANG_OK,
		"Shader does not contain a fragment entry point or it is not named and marked correctly.\n"
		"The fragment entry point must be:\n"
		"\t- Marked with [shader(\"fragment\")]\n"
		"\t- Named FS_main");
	CVLOG("Done.");

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

	//Set up shader object for writing
	libcacaoformats::Shader shader;
	switch(format) {
		case OutputFormat::SPIRV: {
			shader.type = libcacaoformats::Shader::CodeType::SPIRV;
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
					CompileCheck(false, err.str());
				}
			}
			libcacaoformats::Shader::SPIRVCode code(spirv->getBufferSize() / 4);
			std::memcpy(code.data(), spirv->getBufferPointer(), spirv->getBufferSize());
			shader.code = code;
			CVLOG("Done.")
			break;
		}
		case OutputFormat::GLSL: {
			shader.type = libcacaoformats::Shader::CodeType::GLSL;
			ComPtr<slang::IBlob> vertGLSL, fragGLSL;
			{
				CVLOG_NONL("\tGenerating shader vertex stage GLSL... ");
				ComPtr<slang::IBlob> diagnosticsBlob;
				SlangResult r = linked->getEntryPointCode(0, 0, vertGLSL.writeRef(), diagnosticsBlob.writeRef());
				if(r != SLANG_OK || !vertGLSL) {
					std::stringstream err;
					err << "Failed to generate shader GLSL for vertex stage";
					if(diagnosticsBlob) {
						err << ":\n"
							<< (const char*)diagnosticsBlob->getBufferPointer();
					} else {
						err << "!";
					}
					CompileCheck(false, err.str());
				}
				CVLOG("Done.")
			}
			{
				CVLOG_NONL("\tGenerating shader fragment stage GLSL... ");
				ComPtr<slang::IBlob> diagnosticsBlob;
				SlangResult r = linked->getEntryPointCode(1, 0, fragGLSL.writeRef(), diagnosticsBlob.writeRef());
				if(r != SLANG_OK || !fragGLSL) {
					std::stringstream err;
					err << "Failed to generate shader GLSL for fragment stage";
					if(diagnosticsBlob) {
						err << ":\n"
							<< (const char*)diagnosticsBlob->getBufferPointer();
					} else {
						err << "!";
					}
					CompileCheck(false, err.str());
				}
				CVLOG("Done.")
			}
			libcacaoformats::Shader::GLSLCode code;
			code.vertex = std::string((const char*)vertGLSL->getBufferPointer());
			code.fragment = std::string((const char*)fragGLSL->getBufferPointer());
			shader.code = code;
			break;
		}
		default: break;
	}

	//Write shader
	CVLOG_NONL("\tWriting output file " << out << "... ")
	std::ofstream outStream(out);
	CompileCheck(outStream.is_open(), "Failed to open output file!");
	libcacaoformats::PackedEncoder encoder;
	encoder.EncodeShader(shader).ExportToStream(outStream);
	CVLOG("Done.")

	return {true, ""};
}