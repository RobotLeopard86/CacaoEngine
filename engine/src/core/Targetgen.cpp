#include "Targetgen.hpp"

#include "Cacao/Exceptions.hpp"

#include "libcacaoasset.hpp"
#include "slang.h"
#include "slang-com-ptr.h"

#include <sstream>
#include <cstring>
#include <set>

using Slang::ComPtr;

namespace Cacao {
	Slang::ComPtr<slang::IGlobalSession> CompiledShaderObject::gsession;

	std::pair<std::string, std::string> GetEntrypointNames(const libcacaoasset::Shader& in) {
		//Get the names
		std::string vsepName, fsepName;
		if(in.descriptor.domain == libcacaoasset::Shader::Descriptor::Domain::Geometry3D) {
			fsepName = "Cacao_Surface_FSmain";
			switch(in.descriptor.mode) {
				case libcacaoasset::Shader::Descriptor::VertexMode::NoProcess:
					vsepName = "Cacao_NoProcess3D_VSmain";
					break;
				case libcacaoasset::Shader::Descriptor::VertexMode::PreprocessOnly:
					vsepName = "Cacao_Preprocess3D_VSmain";
					break;
				case libcacaoasset::Shader::Descriptor::VertexMode::PostprocessOnly:
					vsepName = "Cacao_Postprocess3D_VSmain";
					break;
				case libcacaoasset::Shader::Descriptor::VertexMode::PreprocessPostprocess:
					vsepName = "Cacao_PrePostprocess3D_VSmain";
					break;
				case libcacaoasset::Shader::Descriptor::VertexMode::Custom:
					vsepName = "Cacao_Custom3D_VSmain";
					break;
			}
		} else {
			fsepName = "Cacao_Canvas_FSmain";
			switch(in.descriptor.mode) {
				case libcacaoasset::Shader::Descriptor::VertexMode::NoProcess:
					vsepName = "Cacao_NoProcess2D_VSmain";
					break;
				case libcacaoasset::Shader::Descriptor::VertexMode::PreprocessOnly:
					vsepName = "Cacao_Preprocess2D_VSmain";
					break;
				case libcacaoasset::Shader::Descriptor::VertexMode::Custom:
					vsepName = "Cacao_Custom2D_VSmain";
					break;
				case libcacaoasset::Shader::Descriptor::VertexMode::PostprocessOnly:
				case libcacaoasset::Shader::Descriptor::VertexMode::PreprocessPostprocess:
					Check<BadValueException>(false, "2D shaders may not use post-processing!");
					throw std::runtime_error("UNREACHABLE CODE!!! HOW DID YOU GET HERE?!");//This will never be reached because of the Check call, but the compiler doesn't know what Check does, so we have to spell it out like it's 3
			}
		}

		//Put 'em in a pair
		return std::make_pair<std::string, std::string>(std::move(vsepName), std::move(fsepName));
	}

	CompiledShaderObject SetupCSO(const libcacaoasset::Shader& in, SlangCompileTarget tgt, const std::string& profile) {
		CompiledShaderObject cso;

		//Describe session
		slang::SessionDesc sessionDesc;
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
		sessionDesc.compilerOptionEntries = entries.data();
		sessionDesc.compilerOptionEntryCount = entries.size();
		slang::TargetDesc tgtDesc = {};
		tgtDesc.format = tgt;
		tgtDesc.profile = CompiledShaderObject::gsession->findProfile(profile.c_str());

		//Create session
		SlangResult r = CompiledShaderObject::gsession->createSession(sessionDesc, cso.session.writeRef());
		Check<ExternalException>(r == SLANG_OK && cso.session, "Failed to create Slang compiler session!");

		//Create Cacao Engine shader base module
		constexpr const char* cacaoModuleSrc =
#include "cacaoengine.inc"
			;
		ComPtr<slang::IModule> cacaoModule;
		{
			ComPtr<slang::IBlob> diagnosticsBlob;
			cacaoModule = cso.session->loadModuleFromSourceString("cacaoengine", "cacaoengine.slang", cacaoModuleSrc, diagnosticsBlob.writeRef());
			if(!cacaoModule) {
				std::stringstream err;
				err << "Failed to create Cacao shader module";
				if(diagnosticsBlob) {
					err << ":\n"
						<< (const char*)diagnosticsBlob->getBufferPointer();
				} else {
					err << "!";
				}
				Check<ExternalException>(false, err.str());
			}
		}

		//Create user module
		ComPtr<slang::IModule> usrModule;
		{
			//Create Slang blob
			slang::IBlob* irBlob = slang_createBlob(in.irCode.data(), in.irCode.size());
			Check<ExternalException>(irBlob != nullptr, "Failed to create IR blob!");
			ComPtr<slang::IBlob> diagnosticsBlob;
			usrModule = cso.session->loadModuleFromIRBlob("cacaousercode", "usercode.slang", irBlob, diagnosticsBlob.writeRef());
			if(!usrModule) {
				std::stringstream err;
				err << "Failed to create user code shader module";
				if(diagnosticsBlob) {
					err << ":\n"
						<< (const char*)diagnosticsBlob->getBufferPointer();
				} else {
					err << "!";
				}
				Check<ExternalException>(false, err.str());
			}
		}

		//Some processing on the pipelines (to add definitions for unused functions)
		constexpr const char* vpipelineModuleSrc =
#include "vertex_pipeline.inc"
			;
		constexpr const char* fpipelineModuleSrc =
#include "fragment_pipeline.inc"
			;
		std::string vpipelineSrc(vpipelineModuleSrc);
		std::string fpipelineSrc(fpipelineModuleSrc);
		std::vector<uint8_t> undefFuncIDs {0, 1, 2, 3, 4};
		constexpr std::array<const char*, 5> undefFuncs {{"void CacaoVertexPreprocess(const Cacao::VSIn3D input, inout Cacao::VSIn3D output) {}",
			"void CacaoVertexPostprocess(const Cacao::VSOut3D input, inout Cacao::VSOut3D output) {}",
			"void CacaoVertexCustom(const Cacao::VSIn3D input, inout Cacao::VSOut3D output) {}",
			"void CacaoVertexPreprocess2D(const Cacao::VSIn2D input, inout Cacao::VSIn2D output) {}",
			"void CacaoVertexCustom2D(const Cacao::VSIn2D input, inout Cacao::VSOut2D output) {}"}};
		if(in.descriptor.domain == libcacaoasset::Shader::Descriptor::Domain::Geometry3D) {
			fpipelineSrc += "\n\nfloat4 CacaoCanvasMain(const Cacao::CanvasInput input) { return float4(1.0); }";
			switch(in.descriptor.mode) {
				case libcacaoasset::Shader::Descriptor::VertexMode::NoProcess:
					break;
				case libcacaoasset::Shader::Descriptor::VertexMode::PreprocessOnly:
					undefFuncIDs.erase(undefFuncIDs.begin());
					break;
				case libcacaoasset::Shader::Descriptor::VertexMode::PostprocessOnly:
					undefFuncIDs.erase(undefFuncIDs.begin() + 1);
					break;
				case libcacaoasset::Shader::Descriptor::VertexMode::PreprocessPostprocess:
					undefFuncIDs.erase(undefFuncIDs.begin());
					undefFuncIDs.erase(undefFuncIDs.begin());
					break;
				case libcacaoasset::Shader::Descriptor::VertexMode::Custom:
					undefFuncIDs.erase(undefFuncIDs.begin() + 2);
					break;
			}
		} else {
			fpipelineSrc += "\n\nvoid CacaoSurfaceMain(const Cacao::SurfaceInput input, inout Cacao::SurfaceSample output) {}";
			switch(in.descriptor.mode) {
				case libcacaoasset::Shader::Descriptor::VertexMode::NoProcess:
					break;
				case libcacaoasset::Shader::Descriptor::VertexMode::PreprocessOnly:
					undefFuncIDs.erase(undefFuncIDs.begin() + 3);
					break;
				case libcacaoasset::Shader::Descriptor::VertexMode::Custom:
					undefFuncIDs.erase(undefFuncIDs.begin() + 4);
					break;
				default: break;
			}
		}
		for(uint8_t id : undefFuncIDs) {
			vpipelineSrc += "\n\n";
			vpipelineSrc += undefFuncs[id];
		}

		//Create Cacao Engine pipelines module
		ComPtr<slang::IModule> vpipelineModule, fpipelineModule;
		{
			ComPtr<slang::IBlob> diagnosticsBlob;
			vpipelineModule = cso.session->loadModuleFromSourceString("vertex_pipeline", "vertex_pipeline.slang", vpipelineSrc.c_str(), diagnosticsBlob.writeRef());
			if(!vpipelineModule) {
				std::stringstream err;
				err << "Failed to create engine vertex pipeline module";
				if(diagnosticsBlob) {
					err << ":\n"
						<< (const char*)diagnosticsBlob->getBufferPointer();
				} else {
					err << "!";
				}
				Check<ExternalException>(false, err.str());
			}
		}
		{
			ComPtr<slang::IBlob> diagnosticsBlob;
			fpipelineModule = cso.session->loadModuleFromSourceString("fragment_pipeline", "fragment_pipeline.slang", fpipelineSrc.c_str(), diagnosticsBlob.writeRef());
			if(!fpipelineModule) {
				std::stringstream err;
				err << "Failed to create engine fragment pipeline module";
				if(diagnosticsBlob) {
					err << ":\n"
						<< (const char*)diagnosticsBlob->getBufferPointer();
				} else {
					err << "!";
				}
				Check<ExternalException>(false, err.str());
			}
		}

		//Get entry points
		ComPtr<slang::IEntryPoint> vsep, fsep;
		auto [vsepName, fsepName] = GetEntrypointNames(in);
		Check<ExternalException>(vpipelineModule->findEntryPointByName(vsepName.c_str(), vsep.writeRef()) == SLANG_OK, "Failed to fetch vertex stage entrypoint!");
		Check<ExternalException>(fpipelineModule->findEntryPointByName(fsepName.c_str(), fsep.writeRef()) == SLANG_OK, "Failed to fetch fragment stage entrypoint!");

		//Compose program
		std::array<slang::IComponentType*, 6> componentTypes {
			cacaoModule,
			usrModule,
			vpipelineModule,
			fpipelineModule,
			vsep,
			fsep};
		ComPtr<slang::IComponentType> composed;
		{
			ComPtr<slang::IBlob> diagnosticsBlob;
			SlangResult r = cso.session->createCompositeComponentType(componentTypes.data(), componentTypes.size(), composed.writeRef(), diagnosticsBlob.writeRef());
			if(r != SLANG_OK || !composed) {
				std::stringstream err;
				err << "Failed to compose shader program";
				if(diagnosticsBlob) {
					err << ":\n"
						<< (const char*)diagnosticsBlob->getBufferPointer();
				} else {
					err << "!";
				}
				Check<ExternalException>(false, err.str());
			}
		}

		//Link shader program
		{
			ComPtr<slang::IBlob> diagnosticsBlob;
			SlangResult r = composed->link(cso.linked.writeRef(), diagnosticsBlob.writeRef());
			if(r != SLANG_OK || !cso.linked) {
				std::stringstream err;
				err << "Failed to link shader program";
				if(diagnosticsBlob) {
					err << ":\n"
						<< (const char*)diagnosticsBlob->getBufferPointer();
				} else {
					err << "!";
				}
				Check<ExternalException>(false, err.str());
			}
		}

		//Return result
		return cso;
	}

	std::vector<uint32_t> GenerateSPV(const libcacaoasset::Shader& in) {
		//Create CSO
		CompiledShaderObject cso = SetupCSO(in, SlangCompileTarget::SLANG_SPIRV, "spirv_1_3");

		//Generate target code
		ComPtr<slang::IBlob> codeBlob;
		{
			ComPtr<slang::IBlob> diagnosticsBlob;
			SlangResult r = cso.linked->getTargetCode(0, codeBlob.writeRef(), diagnosticsBlob.writeRef());
			if(r != SLANG_OK || !codeBlob) {
				std::stringstream err;
				err << "Failed to generate SPIR-V target code";
				if(diagnosticsBlob) {
					err << ":\n"
						<< (const char*)diagnosticsBlob->getBufferPointer();
				} else {
					err << "!";
				}
				Check<ExternalException>(false, err.str());
			}
		}

		//Copy SPIR-V into buffer and return
		std::vector<uint32_t> code(codeBlob->getBufferSize());
		std::memcpy(code.data(), codeBlob->getBufferPointer(), codeBlob->getBufferSize());
		return code;
	}
}