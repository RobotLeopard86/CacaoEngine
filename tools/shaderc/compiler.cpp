#include "compiler.hpp"
#include "toolutil.hpp"

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

	//Link program (we don't store this, we just need to ensure it links to avoid errors at runtime)
	CVLOG_NONL("\tLinking shader program... ");
	{
		ComPtr<slang::IComponentType> linked;
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
	CVLOG_NONL("\tSerializing IR blob... ")
	ComPtr<ISlangBlob> irBlob;
	{
		SlangResult r = mod->serialize(irBlob.writeRef());
		if(r != SLANG_OK || !irBlob) {
			std::stringstream err;
			err << "Failed to serialize IR blob!";
			CompileCheck(false, err.str());
		}
	}
	std::vector<unsigned char> shader(irBlob->getBufferSize());
	std::memcpy(shader.data(), irBlob->getBufferPointer(), irBlob->getBufferSize());
	CVLOG("Done.")

	//Write shader
	CVLOG_NONL("\tWriting output file " << out << "... ");
	std::ofstream outStream(out, std::ios::binary);
	CompileCheck(outStream.is_open(), "Failed to open output file!");
	libcacaoformats::PackedEncoder encoder;
	encoder.EncodeShader(shader).ExportToStream(outStream);
	CVLOG("Done.");

	return {true, ""};
}