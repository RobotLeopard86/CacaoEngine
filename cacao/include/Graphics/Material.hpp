#pragma once

#include "Shader.hpp"
#include "Textures/Texture2D.hpp"
#include "Textures/Cubemap.hpp"
#include "UI/UIView.hpp"
#include "Core/Exception.hpp"

#include "glm/glm.hpp"

#include <variant>

namespace Cacao {
	/**
	 * @brief A shader and arguments to it to draw with
	 *
	 * @note Can only be legally created by Shader::CreateMaterial
	 */
	class Material {
	  public:
		///@brief Shorthand for container of possible data types
		using ValueContainer = std::variant<int, unsigned int, float,
			glm::vec2, glm::vec3, glm::vec4,
			glm::mat2, glm::mat2x3, glm::mat2x4,
			glm::mat3x2, glm::mat3, glm::mat3x4,
			glm::mat4x2, glm::mat4x3, glm::mat4,
			RawTexture*, Texture*, AssetHandle<Cubemap>, AssetHandle<Texture2D>, std::shared_ptr<UIView>>;

		/**
		 * @brief Get the associated shader object
		 *
		 * @return A handle to the shader object or a null handle if the backup raw shader is in use
		 */
		AssetHandle<Shader> GetShader() {
			return shader;
		}

		/**
		 * @brief Write a value into the material to be used
		 *
		 * @param key The key to write to. Must be a valid entry in the associated shader spec.
		 * @param val The value to write
		 *
		 * @throws Exception If the key is not a valid entry in the shader spec
		 */
		void WriteValue(std::string key, ValueContainer val) {
			if(!values.contains(key)) {
				const ShaderSpec spec = shader->GetSpec();
				CheckException(std::find_if(spec.cbegin(), spec.cend(), [key](const ShaderItemInfo& sii) {
					return sii.name.compare(key) == 0;
				}) != spec.cend(),
					Exception::GetExceptionCodeFromMeaning("ContainerValue"), "The requested key is not present in the material!");
			}
			values.insert_or_assign(key, val);
		}

		/**
		 * @brief Read a value from the material
		 *
		 * @param key The key to read. Must be a valid entry in the associated shader spec.
		 *
		 * @return The value of the key
		 *
		 * @throws Exception If the key is not a valid entry in the shader spec or if the value is not of the type provided as a template
		 */
		template<typename T>
		T ReadValue(std::string key) {
			CheckException(values.contains(key), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "The requested key is not present in the material!");

			ValueContainer v = values.at(key);
			try {
				return std::get<T>(v);
			} catch(std::bad_variant_access&) {
				CheckException(false, Exception::GetExceptionCodeFromMeaning("WrongType"), "The requested material value is not of the requested type!");
			}
		}

		/**
		 * @brief Set this material as the active material for drawing
		 *
		 * @note For use by the engine only
		 *
		 * @throws Exception If the material is already active, the shader is not compiled, or if not called on the engine thread
		 */
		void Activate();

		/**
		 * @brief Unset this material as the active material for drawing
		 *
		 * @note For use by the engine only
		 *
		 * @throws Exception If the material is not active or if not called on the engine thread
		 */
		void Deactivate();

		/**
		 * @brief Check if the material is the active one
		 *
		 * @return Whether the material is the active one or not
		 */
		bool IsActiveMaterial() {
			return active;
		}

		~Material();

		Material(const Material&);
		Material& operator=(const Material&);
		Material(Material&&) = delete;
		Material& operator=(Material&&) = delete;

	  private:
		//Chill constructor
		Material(AssetHandle<Shader> shader);

		//Scary raw constructor
		Material(Shader* rawShader);

		//The shader to use
		AssetHandle<Shader> shader;

		//Backup shader pointer in case the shader isn't in the asset cache (DANGER)
		Shader* rawShader;

		//Common initialization
		void _CommonInit();

		//Backend-implemented data type
		struct MaterialData;
		std::shared_ptr<MaterialData> nativeData;

		//Data map
		std::map<std::string, ValueContainer> values;

		//If the material is active
		bool active;

		friend class Shader;
	};
}