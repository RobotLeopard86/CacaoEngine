#pragma once

#include "DllHelper.hpp"
#include "Asset.hpp"

#include "libcacaoasset.hpp"

namespace Cacao {
	/**
	 * @brief Asset type for GPU shaders
	 */
	class CACAO_API Shader : public Asset {
	  public:
		/**
		 * @brief Create a new shader from IR code
		 *
		 * @param shaderIR The Slang IR code to create the shader with
		 * @param desc Shader behavior and input information
		 * @param addr The resource address to associate with the shader
		 *
		 * @throws BadValueException If the IR code buffer is empty
		 * @throws BadValueException If the shader description contains invalid data
		 * @throws BadValueException If the address is malformed
		 */
		static std::shared_ptr<Shader> Create(std::vector<unsigned char>&& shaderIR, libcacaoasset::Shader::Descriptor desc, const std::string& addr);

		///@cond
		Shader(const Shader&) = delete;
		Shader(Shader&&);
		Shader& operator=(const Shader&) = delete;
		Shader& operator=(Shader&&);
		///@endcond

		/**
		 * @brief Convert the shader data into a form suitable for rendering
		 *
		 * @throws BadRealizeStateException If the shader is already realized
		 * @throws BadInitStateException If the graphics backend is not initialized or connected
		 */
		void Realize();

		/**
		 * @brief Destroy the realized representation of the asset
		 *
		 * @throws BadRealizeStateException If the shader is not realized
		 * @throws BadInitStateException If the graphics backend is not initialized or connected
		 */
		void DropRealized();

		///@cond
		class Impl;
		///@endcond

		~Shader();

	  private:
		Shader(std::vector<unsigned char>&& shaderIR, libcacaoasset::Shader::Descriptor desc, const std::string& addr);
		friend class ResourceManager;
		friend class PAL;

		std::unique_ptr<Impl> impl;
		friend class ImplAccessor;
	};
}