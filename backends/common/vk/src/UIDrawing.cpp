#include "UI/Text.hpp"
#include "UI/Image.hpp"
#include "Core/Exception.hpp"
#include "UI/Shaders.hpp"
#include "VkUIView.hpp"
#include "VkUtils.hpp"
#include "ActiveItems.hpp"
#include "VulkanCoreObjects.hpp"

//Special value that helps align single-line text to the anchor point properly (it looks too high otherwise)
#define SINGLE_LINE_ALIGNMENT (float(screenSize.y) * (tr->linegap / 2))

namespace Cacao {
	void PreprocessTextRenderable(Text::Renderable* tr, glm::uvec2 screenSize) {
		std::vector<CharacterInfo> infos;
		int lineCounter = 0;
		for(auto ln : tr->lines) {
			//Calculate starting position
			float startX = -((signed int)tr->size.x / 2);
			float startY = (tr->lines.size() > 1 ? (tr->lineHeight * lineCounter) : SINGLE_LINE_ALIGNMENT);
			if(tr->alignment != TextAlign::Left) {
				float textWidth = 0.0f;
				for(unsigned int i = 0; i < ln.glyphCount; ++i) {
					textWidth += (ln.advances[i].adv.x / 64.0f);
				}
				if(tr->alignment == TextAlign::Center) {
					startX = (float(tr->size.x) - textWidth) / 2.0;
				} else {
					startX = float(tr->size.x) - textWidth - (tr->size.x / 2u);
				}
			}
			startX += tr->screenPos.x;
			startY += tr->screenPos.y;
			startY = (screenSize.y - startY);

			//Make glyphs
			float x = startX, y = startY;
			FT_Set_Pixel_Sizes(tr->fontFace, 0, tr->charSize);
			for(unsigned int i = 0; i < ln.glyphCount; i++) {
				FT_Error err = FT_Load_Glyph(tr->fontFace, ln.glyphInfo[i].codepoint, FT_LOAD_RENDER);
				CheckException(!err, Exception::GetExceptionCodeFromMeaning("External"), "Failed to load glyph from font!")

				CharacterInfo info = {};

				//Get bitmap
				FT_Bitmap& bitmap = tr->fontFace->glyph->bitmap;

				//Check for empty glyph
				if(!bitmap.buffer || bitmap.width == 0 || bitmap.rows == 0) {
					//Advance cursor for next glyph and skip
					x += (ln.advances[i].adv.x / 64.0f);
					y += (ln.advances[i].adv.y / 64.0f);
					continue;
				}

				//Calculate vertex data for glyph
				float w = bitmap.width;
				float h = bitmap.rows;
				float xpos = x + tr->fontFace->glyph->bitmap_left;
				float ypos = y - (h - tr->fontFace->glyph->bitmap_top);
				info.vertices[0] = {{xpos, ypos + h, 0.0f}, {0.0f, 0.0f}};
				info.vertices[1] = {{xpos + w, ypos, 0.0f}, {1.0f, 1.0f}};
				info.vertices[2] = {{xpos, ypos, 0.0f}, {0.0f, 1.0f}};
				info.vertices[3] = {{xpos, ypos + h, 0.0f}, {0.0f, 0.0f}};
				info.vertices[4] = {{xpos + w, ypos + h, 0.0f}, {1.0f, 0.0f}};
				info.vertices[5] = {{xpos + w, ypos, 0.0f}, {1.0f, 1.0f}};

				//Create glyph image and image view
				vk::ImageCreateInfo texCI({}, vk::ImageType::e2D, vk::Format::eR8Unorm, {bitmap.width, bitmap.rows, 1}, 1, 1, vk::SampleCountFlagBits::e1,
					vk::ImageTiling::eLinear, vk::ImageUsageFlagBits::eSampled, vk::SharingMode::eExclusive, 0);
				vma::AllocationCreateInfo texAllocCI {};
				texAllocCI.requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible;
				vma::Allocation alloc;
				try {
					auto [image, _alloc] = allocator.createImage(texCI, texAllocCI);
					alloc = _alloc;
					info.glyph.image = {.alloc = alloc, .obj = image};
					vk::ImageViewCreateInfo viewCI({}, image, vk::ImageViewType::e2D, vk::Format::eR8Unorm,
						{vk::ComponentSwizzle::eZero, vk::ComponentSwizzle::eZero, vk::ComponentSwizzle::eZero, vk::ComponentSwizzle::eR},
						{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
					info.glyph.view = dev.createImageView(viewCI);
				} catch(vk::SystemError& verr) {
					Logging::EngineLog(verr.what(), LogLevel::Error);
				}

				//Copy image data to GPU
				void* gpuMem;
				allocator.mapMemory(alloc, &gpuMem);
				std::memcpy(gpuMem, bitmap.buffer, bitmap.width * bitmap.rows);
				allocator.unmapMemory(alloc);

				//Transition image to shader resource state
				{
					vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eNone,
						vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eTransferWrite,
						vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, 0, 0, info.glyph.image.obj, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
					vk::DependencyInfo cdDI({}, {}, {}, barrier);
					uiCmd->pipelineBarrier2(cdDI);
				}

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
		hb_font_destroy(tr->hbf);

		//Push character infos
		trCharInfos.insert_or_assign(tr, infos);
	}

	void Text::Renderable::Draw(glm::uvec2 screenSize, const glm::mat4& projection) {
		CheckException(trCharInfos.contains(this), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Cannot draw text renderable that hasn't been preprocessed!")

		//Create temporary Vulkan objects
		Allocated<vk::Buffer> vertex;

		//Create buffer
		vk::BufferCreateInfo bufferCI({}, sizeof(UIVertex) * 6, vk::BufferUsageFlagBits::eVertexBuffer, vk::SharingMode::eExclusive);
		vma::AllocationCreateInfo allocCI({}, vma::MemoryUsage::eCpuToGpu);
		{
			auto [buf, alloc] = allocator.createBuffer(bufferCI, allocCI);
			vertex = {.alloc = alloc, .obj = buf};
		}

		//Fetch text character infos
		std::vector<CharacterInfo> infos = trCharInfos.at(this);

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
			RawVkTexture upTex = {.view = character.glyph.view, .slot = new int(-1)};
			up.emplace_back(ShaderUploadItem {.target = "glyph", .data = std::any(upTex)});
			up.emplace_back(ShaderUploadItem {.target = "color", .data = std::any(color)});
			TextShaders::shader->UploadData(up);

			//Draw glyph
			constexpr std::array<vk::DeviceSize, 1> offsets = {{0}};
			uiCmd->bindVertexBuffers(0, vertex.obj, offsets);
			uiCmd->draw(6, 1, 0, 0);

			//Unbind texture
			vk::DescriptorImageInfo dii(VK_NULL_HANDLE, VK_NULL_HANDLE, vk::ImageLayout::eUndefined);
			vk::WriteDescriptorSet wds(VK_NULL_HANDLE, *(upTex.slot), 0, vk::DescriptorType::eCombinedImageSampler, dii);
			uiCmd->pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, activeShader->pipelineLayout, 0, wds);
			delete upTex.slot;
		}

		//Unbind text shader
		TextShaders::shader->Unbind();

		//Unmap vertex buffer
		allocator.unmapMemory(vertex.alloc);

		//Add objects to allocated list
		allocatedObjects.emplace_back(vertex);
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