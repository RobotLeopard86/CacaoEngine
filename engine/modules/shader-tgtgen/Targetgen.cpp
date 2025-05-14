#include "Targetgen.hpp"
#include "CSO.hpp"

#include "libcacaoformats.hpp"

#include <sstream>
#include <atomic>

namespace Targetgen {
	//Slang is annoying about blobs; you can't make one on your own
	//So this is a hacked together mess blob subclass
	//Please fix your API, Slang.
	class VecBlob : public ISlangBlob {
	  public:
		SLANG_NO_THROW void const* SLANG_MCALL getBufferPointer() SLANG_OVERRIDE {
			return data.data();
		}
		SLANG_NO_THROW size_t SLANG_MCALL getBufferSize() SLANG_OVERRIDE {
			return data.size();
		}
		SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE {
			assert(refCount != 0);
			const uint32_t count = --refCount;
			if(count == 0) {
				delete this;
			}
			return count;
		}
		SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE {
			return ++refCount;
		}
		SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const&, void**) SLANG_OVERRIDE {
			return SLANG_E_NO_INTERFACE;
		}
		VecBlob& operator=(const VecBlob&) {
			return *this;
		}

		static inline ComPtr<ISlangBlob> create(const std::vector<unsigned char>& v) {
			return ComPtr<ISlangBlob>(new VecBlob(v));
		}

	  private:
		const std::vector<unsigned char> data;
		std::atomic<uint32_t> refCount;

		VecBlob(const std::vector<unsigned char>& v)
		  : data(v), refCount(0) {}
	};

	CompiledShaderObject SetupCSO(ibytestream& in, SlangCompileTarget tgt, const std::string& profile) {
		CompiledShaderObject cso;

		//Configure global session
		SlangResult r = slang::createGlobalSession(cso.gsession.writeRef());
		CheckException(r == SLANG_OK && cso.gsession, "Failed to create global session!");

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
		CheckException(r == SLANG_OK && cso.session, "Failed to create Slang compiler session!");

		//Create Cacao Engine shader base module
		constexpr const char* cacaoModuleSrc =
#include "cacaoshaderbase.inc"
			;
		ComPtr<slang::IModule> cacaoModule;
		{
			ComPtr<slang::IBlob> diagnosticsBlob;
			cacaoModule = cso.session->loadModuleFromSourceString("cacaoshaderbase", "cacaoshaderbase.slang", cacaoModuleSrc, diagnosticsBlob.writeRef());
			if(!cacaoModule) {
				std::stringstream err;
				err << "Failed to create Cacao shader module";
				if(diagnosticsBlob) {
					err << ":\n"
						<< (const char*)diagnosticsBlob->getBufferPointer();
				} else {
					err << "!";
				}
				CheckException(false, err.str());
			}
		}

		//Create user module
		ComPtr<slang::IModule> usrModule;
		{
			libcacaoformats::PackedDecoder pdec;
			libcacaoformats::PackedContainer pc = libcacaoformats::PackedContainer::FromStream(in);
			std::vector<unsigned char> irData = pdec.DecodeShader(pc);
			ComPtr<slang::IBlob> irBlob = VecBlob::create(irData);

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
				CheckException(false, err.str());
			}
		}

		//Get entry points
		ComPtr<slang::IEntryPoint> vsep, fsep;
		CheckException(usrModule->findEntryPointByName("VS_main", vsep.writeRef()) == SLANG_OK, "Failed to fetch vertex stage entrypoint!");
		CheckException(usrModule->findEntryPointByName("FS_main", fsep.writeRef()) == SLANG_OK, "Failed to fetch fragment stage entrypoint!");

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
				CheckException(false, err.str());
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
				CheckException(false, err.str());
			}
		}
	}
}