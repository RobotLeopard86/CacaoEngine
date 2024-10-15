#include "UI/Text.hpp"
#include "UI/Image.hpp"
#include "Core/Exception.hpp"
#include "UI/Shaders.hpp"
#include "VkUIView.hpp"
#include "VkUtils.hpp"
#include "ActiveItems.hpp"
#include "VulkanCoreObjects.hpp"

//Special value that helps align single-line text to the anchor point properly (it looks too high otherwise)
#define SINGLE_LINE_ALIGNMENT (float(screenSize.y) * (linegap / 2))

namespace Cacao {
	struct UIVertex {
		glm::vec3 vert;
		glm::vec2 tc;
	};

	void Text::Renderable::Draw(glm::uvec2 screenSize, const glm::mat4& projection) {
		CheckException(activeFrame, Exception::GetExceptionCodeFromMeaning("NullValue"), "Cannot draw text renderable when there is no active frame!")

		//Create temporary Vulkan objects
		Allocated<vk::Buffer> vertex;
		vk::Sampler sampler;

		//Create buffer
		vk::BufferCreateInfo bufferCI({}, sizeof(UIVertex) * 6, vk::BufferUsageFlagBits::eVertexBuffer, vk::SharingMode::eExclusive);
		vma::AllocationCreateInfo allocCI({}, vma::MemoryUsage::eCpuToGpu);
		{
			auto [buf, alloc] = allocator.createBuffer(bufferCI, allocCI);
			vertex = {.alloc = alloc, .obj = buf};
		}

		//Create sampler
		vk::SamplerCreateInfo samplerCI({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge,
			vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0.0f, VK_FALSE, 0.0f, VK_FALSE, vk::CompareOp::eNever,
			0.0f, VK_REMAINING_MIP_LEVELS, vk::BorderColor::eIntTransparentBlack, VK_FALSE);
		sampler = dev.createSampler(samplerCI);

		//Create text character infos
		struct CharacterInfo {
			std::array<UIVertex, 6> vertices;
			struct Glyph {
				Allocated<vk::Image> image;
				vk::ImageView view;
			} glyph;
		};
		std::vector<CharacterInfo> infos;
		int lineCounter = 0;
		for(auto ln : lines) {
			//Calculate starting position
			float startX = -((signed int)size.x / 2);
			float startY = (lines.size() > 1 ? (lineHeight * lineCounter) : SINGLE_LINE_ALIGNMENT);
			if(alignment != TextAlign::Left) {
				float textWidth = 0.0f;
				for(unsigned int i = 0; i < ln.glyphCount; ++i) {
					textWidth += (ln.advances[i].adv.x / 64.0f);
				}
				if(alignment == TextAlign::Center) {
					startX = (float(size.x) - textWidth) / 2.0;
				} else {
					startX = float(size.x) - textWidth - (size.x / 2u);
				}
			}
			startX += screenPos.x;
			startY += screenPos.y;
			startY = (screenSize.y - startY);

			//Make glyphs
			float x = startX, y = startY;
			FT_Set_Pixel_Sizes(fontFace, 0, charSize);
			for(unsigned int i = 0; i < ln.glyphCount; i++) {
				FT_Error err = FT_Load_Glyph(fontFace, ln.glyphInfo[i].codepoint, FT_LOAD_RENDER);
				CheckException(!err, Exception::GetExceptionCodeFromMeaning("External"), "Failed to load glyph from font!")

				CharacterInfo info = {};

				//Get bitmap
				FT_Bitmap& bitmap = fontFace->glyph->bitmap;

				//Calculate vertex data for glyph
				float w = bitmap.width;
				float h = bitmap.rows;
				float xpos = x + fontFace->glyph->bitmap_left;
				float ypos = y - (h - fontFace->glyph->bitmap_top);
				info.vertices[0] = {{xpos, ypos + h, 0.0f}, {0.0f, 0.0f}};
				info.vertices[1] = {{xpos + w, ypos, 0.0f}, {1.0f, 1.0f}};
				info.vertices[2] = {{xpos, ypos, 0.0f}, {0.0f, 1.0f}};
				info.vertices[3] = {{xpos, ypos + h, 0.0f}, {0.0f, 0.0f}};
				info.vertices[4] = {{xpos + w, ypos + h, 0.0f}, {1.0f, 0.0f}};
				info.vertices[5] = {{xpos + w, ypos, 0.0f}, {1.0f, 1.0f}};

				//Create glyph image and image view
				vk::ImageCreateInfo texCI({}, vk::ImageType::e2D, vk::Format::eR8Unorm, {bitmap.width, bitmap.rows, 1}, 1, 1, vk::SampleCountFlagBits::e1,
					vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive, 0);
				vma::AllocationCreateInfo texAllocCI({}, vma::MemoryUsage::eAuto, vk::MemoryPropertyFlagBits::eHostVisible);
				auto [image, alloc] = allocator.createImage(texCI, texAllocCI);
				info.glyph.image = {.alloc = alloc, .obj = image};
				vk::ImageViewCreateInfo viewCI({}, image, vk::ImageViewType::e2D, vk::Format::eR8Unorm,
					{vk::ComponentSwizzle::eZero, vk::ComponentSwizzle::eZero, vk::ComponentSwizzle::eZero, vk::ComponentSwizzle::eR},
					{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
				info.glyph.view = dev.createImageView(viewCI);

				//Copy image data to GPU
				void* gpuMem;
				allocator.mapMemory(alloc, &gpuMem);
				std::memcpy(gpuMem, bitmap.buffer, bitmap.width * bitmap.rows);
				allocator.unmapMemory(alloc);

				infos.push_back(std::move(info));

				//Advance cursor for next glyph
				x += (ln.advances[i].adv.x / 64.0f);
				y += (ln.advances[i].adv.y / 64.0f);
			}

			//Destroy Harfbuzz buffer
			hb_buffer_destroy(ln.buffer);

			//Increment counter
			lineCounter++;
		}

		//Destroy Harfbuzz font representation
		hb_font_destroy(hbf);

		//Map vertex buffer memory
		void* vertexBuffer;
		allocator.mapMemory(vertex.alloc, &vertexBuffer);

		//Bind text shader
		TextShaders::shader->Bind();

		//Draw characters
		for(const CharacterInfo& character : infos) {
			//Copy vertex data into buffer
			std::memcpy(vertexBuffer, character.vertices.data(), sizeof(UIVertex) * 6);

			//Upload uniform data
			ShaderUploadData up;
			RawVkTexture upTex = {.view = character.glyph.view, .sampler = sampler, .slot = new int(-1)};
			up.emplace_back(ShaderUploadItem {.target = "glyph", .data = std::any(upTex)});
			up.emplace_back(ShaderUploadItem {.target = "color", .data = std::any(color)});
			TextShaders::shader->UploadData(up);

			//Draw glyph
			constexpr std::array<vk::DeviceSize, 1> offsets = {{0}};
			uiCmd->bindVertexBuffers(0, vertex.obj, offsets);
			uiCmd->draw(6, 1, 0, 0);

			//Unbind texture
			vk::DescriptorImageInfo dii(VK_NULL_HANDLE, VK_NULL_HANDLE, vk::ImageLayout::eUndefined);
			vk::WriteDescriptorSet wds(activeShader->dset, *(upTex.slot), 0, vk::DescriptorType::eCombinedImageSampler, dii);
			dev.updateDescriptorSets(wds, {});
			delete upTex.slot;
		}

		//Unbind text shader
		TextShaders::shader->Unbind();

		//Unmap vertex buffer
		allocator.unmapMemory(vertex.alloc);

		//Add objects to allocated list
		allocatedObjects.emplace_back(vertex);
		allocatedObjects.emplace_back(sampler);
		for(const CharacterInfo& character : infos) {
			allocatedObjects.emplace_back(character.glyph.view);
			allocatedObjects.emplace_back(character.glyph.image);
		}
	}

	void Image::Renderable::Draw(glm::uvec2 screenSize, const glm::mat4& projection) {
		//Get position of top left
		glm::uvec2 topLeft = screenPos - (size / 2u);
		topLeft.y = screenSize.y - topLeft.y;

		//Create vertex buffer
		Allocated<vk::Buffer> vertex;
		UIVertex vertexData[6] = {
			{{topLeft.x, topLeft.y - size.y, 0.0f}, {0.0f, 0.0f}},
			{{topLeft.x + size.x, topLeft.y, 0.0f}, {1.0f, 1.0f}},
			{{topLeft.x, topLeft.y, 0.0f}, {0.0f, 1.0f}},
			{{topLeft.x, topLeft.y - size.y, 0.0f}, {0.0f, 0.0f}},
			{{topLeft.x + size.x, topLeft.y - size.y, 0.0f}, {1.0f, 0.0f}},
			{{topLeft.x + size.x, topLeft.y, 0.0f}, {1.0f, 1.0f}}};
		vk::BufferCreateInfo bufferCI({}, sizeof(UIVertex) * 6, vk::BufferUsageFlagBits::eVertexBuffer, vk::SharingMode::eExclusive);
		vma::AllocationCreateInfo allocCI({}, vma::MemoryUsage::eCpuToGpu);
		{
			auto [buf, alloc] = allocator.createBuffer(bufferCI, allocCI);
			vertex = {.alloc = alloc, .obj = buf};
		}
		void* vertexBuffer;
		allocator.mapMemory(vertex.alloc, &vertexBuffer);
		std::memcpy(vertexBuffer, vertexData, sizeof(UIVertex) * 6);
		allocator.unmapMemory(vertex.alloc);

		//Upload uniforms
		ImageShaders::shader->Bind();
		ShaderUploadData up;
		up.emplace_back(ShaderUploadItem {.target = "image", .data = std::any(tex)});
		ImageShaders::shader->UploadData(up);

		//Draw image
		constexpr std::array<vk::DeviceSize, 1> offsets = {{0}};
		uiCmd->bindVertexBuffers(0, vertex.obj, offsets);
		uiCmd->draw(6, 1, 0, 0);

		//Unbind objects
		tex->Unbind();
		ImageShaders::shader->Unbind();

		//Add vertex buffer to allocated list
		allocatedObjects.emplace_back(vertex);
	}

}