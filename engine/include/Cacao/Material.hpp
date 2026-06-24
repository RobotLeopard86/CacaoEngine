#pragma once

#include "DllHelper.hpp"
#include "Asset.hpp"
#include "Shader.hpp"

#include "libcacaoasset.hpp"

namespace Cacao {
	/**
	 * @brief Asset type for rendering materials
	 */
	class CACAO_API Material : public Asset {
	  public:
		/**
		 * @brief Create a new material based on a shader
		 *
		 * @param shader The shader asset to use when rendering
		 * @param addr The resource address to associate with the material
		 *
		 * @throws BadValueException If the provided shader pointer is an invalid asset
		 * @throws BadValueException If the address is malformed
		 */
		static std::shared_ptr<Material> Create(std::shared_ptr<Shader> shader, const std::string& addr);

		///@cond
		Material(const Material&) = delete;
		Material(Material&&);
		Material& operator=(const Material&) = delete;
		Material& operator=(Material&&);
		///@endcond

		/**
		 * @brief Convert the material data into a form suitable for rendering
		 *
		 * @throws BadRealizeStateException If the material is already realized
		 * @throws BadInitStateException If the graphics backend is not initialized or connected
		 */
		void Realize();

		/**
		 * @brief Destroy the realized representation of the asset
		 *
		 * @throws BadRealizeStateException If the material is not realized
		 * @throws BadInitStateException If the graphics backend is not initialized or connected
		 */
		void DropRealized();

		///@cond
		class Impl;
		///@endcond

		~Material();

	  private:
		Material(std::shared_ptr<Shader> shader, const std::string& addr);
		friend class ResourceManager;
		friend class PAL;

		std::unique_ptr<Impl> impl;
		friend class ImplAccessor;
	};
}