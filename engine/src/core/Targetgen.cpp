#include "Targetgen.hpp"

#include "Cacao/Exceptions.hpp"

#include "slang.h"
#include "slang-com-ptr.h"

#include <sstream>

using Slang::ComPtr;

namespace Cacao {
	CompiledShaderObject SetupCSO(ibytestream& in, SlangCompileTarget tgt, const std::string& profile) {
		CompiledShaderObject cso;

		//Configure global session
		SlangResult r = slang::createGlobalSession(cso.gsession.writeRef());
		Check<ExternalException>(r == SLANG_OK && cso.gsession, "Failed to create global session!");

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
		tgtDesc.profile = cso.gsession->findProfile(profile.c_str());

		//Create session
		r = cso.gsession->createSession(sessionDesc, cso.session.writeRef());
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
			//Read out input buffer
			std::vector<unsigned char> irData = [&in]() {
				try {
					//Grab size
					in.clear();
					in.exceptions(std::ios::failbit | std::ios::badbit);
					in.seekg(0, std::ios::end);
					auto size = in.tellg();
					in.seekg(0, std::ios::beg);

					//Read data
					std::vector<unsigned char> contents(size);
					in.read(reinterpret_cast<char*>(contents.data()), size);

					return contents;
				} catch(std::ios_base::failure& ios_failure) {
					if(errno == 0) { throw ios_failure; }
					throw std::runtime_error("Failed to read shader bytecode stream!");
				}
			}();
			slang::IBlob* irBlob = slang_createBlob(irData.data(), irData.size());
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

		//Get entry points
		ComPtr<slang::IEntryPoint> vsep, fsep;
		Check<ExternalException>(usrModule->findEntryPointByName("VS_main", vsep.writeRef()) == SLANG_OK, "Failed to fetch vertex stage entrypoint!");
		Check<ExternalException>(usrModule->findEntryPointByName("FS_main", fsep.writeRef()) == SLANG_OK, "Failed to fetch fragment stage entrypoint!");

		//Compose program
		std::array<slang::IComponentType*, 4> componentTypes {
			cacaoModule,
			usrModule,
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

	std::vector<uint32_t> GenerateSPV(ibytestream& in) {
		//Create CSO
		CompiledShaderObject cso = SetupCSO(in, SlangCompileTarget::SLANG_SPIRV, "spirv_1_3");

		//Generate target code
		ComPtr<slang::IBlob> codeBlob;
		{
			ComPtr<slang::IBlob> diagnosticsBlob;
			SlangResult r = cso.linked->getTargetCode(0, codeBlob.writeRef(), diagnosticsBlob.writeRef());
			if(r != SLANG_OK || !codeBlob) {
				std::stringstream err;
				err << "Failed to generate SPIR-V target code!";
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