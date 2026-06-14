#include "compiler.hpp"
#include "slang.h"
#include "toolutil.hpp"

#include "CheckException.hpp"
#include "libcacaoasset.hpp"

#include <sstream>
#include <fstream>
#include <cstring>

CacaoShaderCompiler::CacaoShaderCompiler() {
	//Initialize global session
	CVLOG_NONL("Initializing Slang global session... ");
	SlangResult r = slang::createGlobalSession(gSession.writeRef());
	CheckException(r == SLANG_OK && gSession, "Failed to create Slang global session!");
	CVLOG("Done.");
}

std::pair<bool, std::string> CacaoShaderCompiler::compile(const std::filesystem::path& in, const std::filesystem::path& out) {
	CVLOG_SINGLE("Compiling " << in << ": ")

	//Create session
	CVLOG_NONL("\tCreating compiler session... ");
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
		{
			slang::CompilerOptionEntry opt = {};
			opt.name = slang::CompilerOptionName::VulkanUseEntryPointName;
			opt.value = slang::CompilerOptionValue {slang::CompilerOptionValueKind::Int, true, 0, nullptr, nullptr};
			entries.push_back(opt);
		}
		tgtDesc.format = SLANG_SPIRV;
		tgtDesc.profile = gSession->findProfile("spirv_1_3");
		sessionDesc.targets = &tgtDesc;
		sessionDesc.targetCount = 1;
		sessionDesc.compilerOptionEntries = entries.data();
		sessionDesc.compilerOptionEntryCount = entries.size();
		SlangResult r = gSession->createSession(sessionDesc, session.writeRef());
		CompileCheck(r == SLANG_OK && session, "Failed to create Slang session!");
	}
	CVLOG("Done.");

	constexpr const char* cacaoModuleSrc =
#include "cacaoengine.inc"
		;

	//Load Cacao Engine module
	CVLOG_NONL("\tLoading Cacao shader module... ");
	ComPtr<slang::IModule> cacaoModule;
	{
		ComPtr<slang::IBlob> diagnosticsBlob;
		cacaoModule = session->loadModuleFromSourceString("cacaoengine", "cacaoengine.slang", cacaoModuleSrc, diagnosticsBlob.writeRef());
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
	CVLOG_NONL("\tCompiling user shader module... ");
	ComPtr<slang::IModule> mod;
	{
		ComPtr<slang::IBlob> diagnosticsBlob;
		mod = session->loadModuleFromSourceString(in.string().c_str(), nullptr, src.c_str(), diagnosticsBlob.writeRef());
		if(!mod) {
			std::stringstream err;
			err << "Failed to compile shader module";
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

	//Validate shader
	CVLOG_NONL("\tValidating shader... ");
	slang::ProgramLayout* layout = mod->getLayout(0, nullptr);
	{
		//Validate surface shader function
		slang::FunctionReflection* smain = layout->findFunctionByName("CacaoSurfaceMain");
		CompileCheck(smain != nullptr, "Shader validation error: shader does not contain a surface shader entry point!\n"
									   "Hint: Surface shader entry point must be named CacaoSurfaceMain.");
		CompileCheck(smain->getReturnType()->getScalarType() == slang::TypeReflection::Void, "Shader validation error: surface shader entry point return type is incorrect!\n"
																							 "Hint: Surface shader entry point must return void.");
		CompileCheck(smain->getParameterCount() == 2, "Shader validation error: surface shader entry point takes wrong parameter count!\n"
													  "Hint: Surface shader entry point must take exactly two (2) arguments.");
		slang::VariableReflection* sp1 = smain->getParameterByIndex(0);
		CompileCheck(sp1->findModifier(slang::Modifier::ID::Const) != nullptr, "Shader validation error: surface shader entry point parameter 1 is malformed!\n"
																			   "Hint: Surface shader entry point parameter 1 must be const.");
		ComPtr<slang::IBlob> fullSP1TypeName;
		sp1->getType()->getFullName(fullSP1TypeName.writeRef());
		CompileCheck(!sp1->getType()->isArray() && fullSP1TypeName && std::string((const char*)fullSP1TypeName->getBufferPointer(), fullSP1TypeName->getBufferSize()).compare("Cacao.SurfaceInput") == 0,
			"Shader validation error: surface shader entry point parameter 1 is malformed!\n"
			"Hint: Surface shader entry point parameter 1 must be of type Cacao::SurfaceInput.");
		slang::VariableReflection* sp2 = smain->getParameterByIndex(1);
		CompileCheck(sp2->findModifier(slang::Modifier::ID::InOut) != nullptr, "Shader validation error: surface shader entry point parameter 2 is malformed!\n"
																			   "Hint: Surface shader entry point parameter 2 must be const.");
		ComPtr<slang::IBlob> fullSP2TypeName;
		sp2->getType()->getFullName(fullSP2TypeName.writeRef());
		CompileCheck(!sp2->getType()->isArray() && fullSP2TypeName && std::string((const char*)fullSP2TypeName->getBufferPointer(), fullSP2TypeName->getBufferSize()).compare("Cacao.SurfaceSample") == 0,
			"Shader validation error: surface shader entry point parameter 2 is malformed!\n"
			"Hint: Surface shader entry point parameter 2 must be of type Cacao::SurfaceSample.");
	}
	CVLOG("Done.")

	//Serialize IR blob
	CVLOG_NONL("\tSerializing IR blob... ")
	ComPtr<ISlangBlob>
		irBlob;
	{
		SlangResult r = mod->serialize(irBlob.writeRef());
		if(r != SLANG_OK || !irBlob) {
			std::stringstream err;
			err << "Failed to serialize IR blob!";
			CompileCheck(false, err.str());
		}
	}
	libcacaoasset::Shader shader;
	shader.irCode = std::vector<unsigned char>(irBlob->getBufferSize());
	std::memcpy(shader.irCode.data(), irBlob->getBufferPointer(), irBlob->getBufferSize());
	CVLOG("Done.")

	//Write shader
	CVLOG_NONL("\tWriting output file " << out << "... ");
	std::ofstream outStream(out, std::ios::binary);
	CompileCheck(outStream.is_open(), "Failed to open output file!");
	libcacaoasset::EncodeShader(shader, &outStream);
	CVLOG("Done.");

	return {true, ""};
}