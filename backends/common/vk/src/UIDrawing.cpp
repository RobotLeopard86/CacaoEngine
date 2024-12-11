#include "UI/Text.hpp"
#include "UI/Image.hpp"
#include "Core/Exception.hpp"
#include "UI/Shaders.hpp"
#include "VkUIView.hpp"
#include "VkUtils.hpp"
#include "ActiveItems.hpp"
#include "VulkanCoreObjects.hpp"
#include "UIDrawUBO.hpp"
#include "Graphics/Material.hpp"

#include "glm/gtc/type_ptr.hpp"

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
				CheckException(!err, Exception::GetExceptionCodeFromMeaning("External"), "Failed to load glyph from font!");

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
				info.vertices[0] = {{xpos, ypos, 0.0f}, {0.0f, 1.0f}};
				info.vertices[1] = {{xpos + w, ypos, 0.0f}, {1.0f, 1.0f}};
				info.vertices[2] = {{xpos, ypos + h, 0.0f}, {0.0f, 0.0f}};
				info.vertices[3] = {{xpos + w, ypos, 0.0f}, {1.0f, 1.0f}};
				info.vertices[4] = {{xpos + w, ypos + h, 0.0f}, {1.0f, 0.0f}};
				info.vertices[5] = {{xpos, ypos + h, 0.0f}, {0.0f, 0.0f}};

				//Create glyph image and image view
				vk::ImageCreateInfo texCI({}, vk::ImageType::e2D, vk::Format::eR8Unorm, {bitmap.width, bitmap.rows, 1}, 1, 1, vk::SampleCountFlagBits::e1,
					vk::ImageTiling::eLinear, vk::ImageUsageFlagBits::eSampled, vk::SharingMode::eExclusive, 0);

				vma::AllocationCreateInfo texAllocCI {};
				texAllocCI.requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible;
				vma::Allocation alloc;
				vk::SubresourceLayout subLayout;
				try {
					auto [image, _alloc] = allocator.createImage(texCI, texAllocCI);
					alloc = _alloc;
					info.glyph.image = {.alloc = alloc, .obj = image};
					vk::ImageViewCreateInfo viewCI({}, image, vk::ImageViewType::e2D, vk::Format::eR8Unorm,
						{vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eOne},
						{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
					info.glyph.view = dev.createImageView(viewCI);
					subLayout = dev.getImageSubresourceLayout(image, vk::ImageSubresource(vk::ImageAspectFlagBits::eColor, 0, 0));
				} catch(vk::SystemError& verr) {
					Logging::EngineLog(verr.what(), LogLevel::Error);
				}

				//Copy image data to GPU
				unsigned char* gpuMem;
				allocator.mapMemory(alloc, reinterpret_cast<void**>(&gpuMem));
				for(unsigned int i = 0; i < bitmap.rows; i++) {
					/*
					To explain:
					Linear-tiled images have a row pitch (number of bytes between rows) that typically doesn't align with FreeType's pitch
					What we do here is copy each row of glyph data to the the buffer then skip until we find the next row as Vulkan expects it
					*/
					std::memcpy(gpuMem + (subLayout.rowPitch * i), bitmap.buffer + (bitmap.width * i), bitmap.width);
				}
				allocator.unmapMemory(alloc);

				//Transition image to shader resource state
				{
					vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eNone,
						vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eShaderSampledRead,
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
		CheckException(trCharInfos.contains(this), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Cannot draw text renderable that hasn't been preprocessed!");

		//Fetch text character infos
		std::vector<CharacterInfo> infos = trCharInfos.at(this);

		//Create temporary Vulkan objects
		Allocated<vk::Buffer> vertex;

		//Create buffer
		vk::BufferCreateInfo bufferCI({}, sizeof(UIVertex) * 6 * infos.size(), vk::BufferUsageFlagBits::eVertexBuffer, vk::SharingMode::eExclusive);
		vma::AllocationCreateInfo allocCI({}, vma::MemoryUsage::eCpuToGpu);
		{
			auto [buf, alloc] = allocator.createBuffer(bufferCI, allocCI);
			vertex = {.alloc = alloc, .obj = buf};
		}

		//Map vertex buffer memory
		void* vertexBuffer;
		allocator.mapMemory(vertex.alloc, &vertexBuffer);

		//Create temporary material
		std::shared_ptr<Material> m = TextShaders::shader->CreateMaterial();

		//Write material values (texture is not written because it changes frequently and so must be done manually)
		m->WriteValue("color", color);

		//Activate material
		m->Activate();

		//Bind UI drawing globals UBO
		vk::DescriptorBufferInfo uiDrawGlobalsDBI(uiUBO.obj, 0, vk::WholeSize);
		vk::WriteDescriptorSet dsWrite(VK_NULL_HANDLE, 0, 0, 1, vk::DescriptorType::eUniformBuffer, VK_NULL_HANDLE, &uiDrawGlobalsDBI);
		uiCmd->pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, activeShader->pipelineLayout, 0, dsWrite);

		//Push identity transform matrix
		glm::mat4 identityTransform = glm::identity<glm::mat4>();
		uiCmd->pushConstants(activeShader->pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4), glm::value_ptr(identityTransform));

		//Draw characters
		unsigned int start = 0;
		uiCmd->bindVertexBuffers(0, vertex.obj, {0});
		for(const CharacterInfo& character : infos) {
			//Copy vertex data into buffer
			std::memcpy(static_cast<unsigned char*>(vertexBuffer) + (sizeof(UIVertex) * start), character.vertices.data(), sizeof(UIVertex) * 6);

			//Bind texture
			{
				vk::DescriptorImageInfo dii(VK_NULL_HANDLE, character.glyph.view, vk::ImageLayout::eShaderReadOnlyOptimal);
				vk::WriteDescriptorSet wds(VK_NULL_HANDLE, 2, 0, vk::DescriptorType::eCombinedImageSampler, dii);
				uiCmd->pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, activeShader->pipelineLayout, 0, wds);
			}

			//Draw glyph
			uiCmd->draw(6, 1, start, 0);
			start += 6;

			//Unbind texture
			{
				vk::DescriptorImageInfo dii(VK_NULL_HANDLE, nullView, vk::ImageLayout::eShaderReadOnlyOptimal);
				vk::WriteDescriptorSet wds(VK_NULL_HANDLE, 2, 0, vk::DescriptorType::eCombinedImageSampler, dii);
				uiCmd->pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, activeShader->pipelineLayout, 0, wds);
			}
		}

		//Deactivate material
		m->Deactivate();

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
			{{topLeft.x, topLeft.y, 0.0f}, {0.0f, 1.0f}},
			{{topLeft.x, topLeft.y - size.y, 0.0f}, {0.0f, 0.0f}},
			{{topLeft.x + size.x, topLeft.y, 0.0f}, {1.0f, 1.0f}},
			{{topLeft.x + size.x, topLeft.y, 0.0f}, {1.0f, 1.0f}},
			{{topLeft.x, topLeft.y - size.y, 0.0f}, {0.0f, 0.0f}},
			{{topLeft.x + size.x, topLeft.y - size.y, 0.0f}, {1.0f, 0.0f}},
		};
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

		//Make temporary material
		std::shared_ptr<Material> m = ImageShaders::shader->CreateMaterial();
		m->WriteValue("image", tex);

		//Activate material
		m->Activate();

		//Bind UI drawing globals UBO
		vk::DescriptorBufferInfo uiDrawGlobalsDBI(uiUBO.obj, 0, vk::WholeSize);
		vk::WriteDescriptorSet dsWrite(VK_NULL_HANDLE, 0, 0, 1, vk::DescriptorType::eUniformBuffer, VK_NULL_HANDLE, &uiDrawGlobalsDBI);
		uiCmd->pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, activeShader->pipelineLayout, 0, dsWrite);

		//Draw image
		constexpr std::array<vk::DeviceSize, 1> offsets = {{0}};
		uiCmd->bindVertexBuffers(0, vertex.obj, offsets);
		uiCmd->draw(6, 1, 0, 0);

		//Deactivate material
		m->Deactivate();

		//Add vertex buffer to allocated list
		allocatedObjects.emplace_back(vertex);
	}

}