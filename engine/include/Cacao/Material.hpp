#pragma once

#include "Cacao/Exceptions.hpp"
#include "Cubemap.hpp"
#include "DllHelper.hpp"
#include "Resource.hpp"
#include "Shader.hpp"
#include "Tex2D.hpp"

#include "libcacaoasset.hpp"

#include "glm/glm.hpp"

#include <memory>
#include <variant>

///@cond
template<typename T, typename Variant>
struct is_variant_alternative : std::false_type {};

template<typename T, typename... Ts>
struct is_variant_alternative<T, std::variant<Ts...>>
  : std::bool_constant<(std::same_as<T, Ts> || ...)> {};

template<typename T, typename Variant>
inline constexpr bool is_variant_alternative_v =
	is_variant_alternative<T, Variant>::value;
///@endcond

namespace Cacao {
	/**
	 * @brief Asset type for rendering materials (technically does not inherit from Asset because baking is not applicable)
	 */
	class CACAO_API Material : public Resource {
	  public:
		/**
		 * @brief A storage type that holds any of the valid material parameter data types
		 */
		using ParamValue = std::variant<int, unsigned int, float, bool, glm::ivec2, glm::ivec3, glm::ivec4,
			glm::uvec2, glm::uvec3, glm::uvec4, glm::vec2, glm::vec3, glm::vec4, glm::mat2, glm::mat3, glm::mat4, std::shared_ptr<Tex2D>, std::shared_ptr<Cubemap>>;

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
		 * @brief Get a handle to the shader used by the material
		 *
		 * @return The current shader
		 */
		std::shared_ptr<Shader> GetShader();

		/**
		 * @brief Set the value of a material parameter
		 *
		 * @tparam T The type of the parameter being set; must be one of the types in ParamValue
		 *
		 * @param name The name of the parameter to set
		 * @param value The value to store in the parameter
		 *
		 * @throws NonexistentValueException If the given parameter name is not one in the material
		 * @throws BadTypeException If the type given does not match the parameter type
		 * @throws NonexistentValueException If a texture type is passed and the handle is empty
		 */
		template<typename T>
			requires is_variant_alternative_v<T, ParamValue>
		void SetParameter(const std::string& name, const T& value) {
			_SetParam(name, value);
		}

		/**
		 * @brief Set the value of a material parameter
		 *
		 * @tparam T The type of the parameter being set; must be one of the types in ParamValue
		 *
		 * @param name The name of the parameter to set
		 *
		 * @throws NonexistentValueException If the given parameter name is not one in the material
		 * @throws NonexistentValueException If no value has been set for the given parameter
		 * @throws BadTypeException If the type given does not match the parameter type
		 *
		 * @return The current value of the parameter
		 */
		template<typename T>
			requires is_variant_alternative_v<T, ParamValue>
		T GetParameter(const std::string& name) {
			ParamValue pv = _GetParam(name);
			Check<BadTypeException>(std::holds_alternative<T>(pv), "Cannot get parameter from material with wrong type!");
			return std::get<T>(pv);
		}

		/**
		 * @brief Set the rendering mode
		 *
		 * @param mode The new rendering mode
		 */
		void SetRenderMode(libcacaoasset::Material::RenderMode mode);

		/**
		 * @brief Get the current rendering mode
		 *
		 * @return The rendering mode
		 */
		libcacaoasset::Material::RenderMode GetRenderMode();

		///@cond
		class Impl;
		///@endcond

	  private:
		Material(std::shared_ptr<Shader> shader, const std::string& addr);
		friend class ResourceManager;
		friend class PAL;

		void _SetParam(const std::string& name, const ParamValue& value);
		ParamValue _GetParam(const std::string& name);

		std::unique_ptr<Impl> impl;
		friend class ImplAccessor;
	};
}