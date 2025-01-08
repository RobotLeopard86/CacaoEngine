#pragma once

#include <vector>
#include <filesystem>
#include <map>
#include <future>

#include "Core/Log.hpp"
#include "Core/Assert.hpp"
#include "Core/DllHelper.hpp"
#include "Utilities/MiscUtils.hpp"
#include "Utilities/Asset.hpp"

#include "glm/glm.hpp"

namespace Cacao {
	///@brief Shorthand type for shader data types
	enum class CACAO_API ShaderDataType {
		Unknown = 0,
		Int = 7,
		UInt = 8,
		Float = 13,
		SampledImage = 17,
	};

	///@brief Item in a ShaderSpec
	struct CACAO_API ShaderItemInfo {
		ShaderDataType type;///<Type of data
		glm::uvec2 size;	///<Size (x is columns, y is rows). Example: scalars are {1, 1}, vectors are {size, 1}, matrices are {x, y}
		std::string name;	///<Name of the item in the shader source
	};

	///@brief Collection of shader input items
	using ShaderSpec = std::vector<ShaderItemInfo>;

	///@cond
	class Material;
	///@endcond

	/**
	 * @brief A shader program. Implementation is backend-dependent
	 */
	class CACAO_API Shader final : public Asset {
	  public:
		/**
		 * @brief Create a shader from SPIR-V code
		 *
		 * @param vertex SPIR-V code for the vertex shader
		 * @param fragment SPIR-V code for the vertex shader
		 * @param spec The shader specification
		 *
		 * @note Not recommended for use by games, but if it's necessary to embed SPIR-V in code, go ahead...
		 */
		Shader(std::vector<uint32_t>& vertex, std::vector<uint32_t>& fragment, ShaderSpec spec);

		/**
		 * @brief Create a shader from files
		 *
		 * @param vertex Path to SPIR-V code for the vertex shader
		 * @param fragment Path to SPIR-V code for the fragment shader
		 * @param spec The shader specification
		 *
		 * @note Prefer to use AssetManager::LoadShader over direct construction
		 *
		 * @throws Exception If the files don't exist or could not be opened
		 */
		Shader(std::string vertex, std::string fragment, ShaderSpec spec);

		/**
		 * @brief Delete the shader and its compiled data
		 */
		~Shader() final {
			if(compiled && bound) Unbind();
			if(compiled) Release();
			_BackendDestruct();
		}

		/**
		 * @brief Activate this shader for drawing
		 *
		 * @note For use by the engine only
		 *
		 * @throws Exception If the shader is already bound, not compiled, or if not called on the engine thread
		 */
		void Bind();

		/**
		 * @brief Deactivate this shader for drawing
		 *
		 * @note For use by the engine only
		 *
		 * @throws Exception If the shader is not bound, not compiled, or if not called on the engine thread
		 */
		void Unbind();

		/**
		 * @brief Compile the raw SPIR-V code into a format that can be run by the GPU asynchronously
		 *
		 * @return A future that will resolve when compilation is done
		 *
		 * @throws Exception If the shader was already compiled or the SPIR-V is invalid
		 */
		std::shared_future<void> CompileAsync() override;

		/**
		 * @brief Compile the raw SPIR-V code into a format that can be run by the GPU synchronously
		 *
		 * @throws Exception If the shader was already compiled or the SPIR-V is invalid
		 */
		void CompileSync() override;

		/**
		 * @brief Delete the compiled data
		 *
		 * @throws Exception If the shader was not compiled or is bound
		 */
		void Release() override;

		/**
		 * @brief Check if the shader is bound
		 *
		 * @return If the shader is bound
		 */
		bool IsBound() const {
			return bound;
		}

		/**
		 * @brief Get the shader spec
		 *
		 * @return An immutable reference to the shader specification
		 */
		const ShaderSpec& GetSpec() const {
			return specification;
		}

		/**
		 * @brief Upload engine global data
		 *
		 * @param projection The projection matrix
		 * @param view The view matrix
		 *
		 * @note For use by the engine only
		 */
		static void UploadCacaoGlobals(glm::mat4 projection, glm::mat4 view);

		///@brief Gets the type of this asset. Needed for safe downcasting from Asset
		std::string GetType() const override {
			return "SHADER";
		}

		/**
		 * @brief Create a new material from this shader
		 *
		 * @return A new material that can be used for drawing
		 */
		std::shared_ptr<Material> CreateMaterial();

	  private:
		//Backend-implemented data type
		struct ShaderData;

		bool bound;
		std::shared_ptr<ShaderData> nativeData;
		const ShaderSpec specification;

		//Backend-implemented destruction hook
		void _BackendDestruct();

		friend class Material;
	};
}