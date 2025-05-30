#include "Targetgen.hpp"
#include "CSO.hpp"

#include <sstream>

namespace Targetgen {
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
				err << "Failed to generate target code!";
				if(diagnosticsBlob) {
					err << ":\n"
						<< (const char*)diagnosticsBlob->getBufferPointer();
				} else {
					err << "!";
				}
				CheckException(false, err.str());
			}
		}

		//Copy SPIR-V into buffer and return
		std::vector<uint32_t> code(codeBlob->getBufferSize());
		std::memcpy(code.data(), codeBlob->getBufferPointer(), codeBlob->getBufferSize());
		return code;
	}
}