#include "Cacao/Model.hpp"
#include "Cacao/Exceptions.hpp"

#include "assimp/Importer.hpp"
#include "assimp/config.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/texture.h"
#include "libcacaocommon.hpp"
#include "libcacaoimage.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/rotate_vector.hpp"

#include <unordered_map>

namespace Cacao {
	struct Model::Impl {
		Assimp::Importer importer;
		const aiScene* scene;

		enum class ModelOrientation {
			PosX,
			NegX,
			PosY,
			NegY,
			PosZ,
			NegZ
		} orient;

		std::unordered_map<std::string, aiMesh*> meshIndex;
		std::unordered_map<std::string, aiTexture*> textureIndex;
	};

	Model::Model(std::vector<unsigned char>&& modelBin, const std::string& addr)
	  : Resource(addr) {
		Check<BadValueException>(ValidateResourceAddr<Model>(addr), "Resource address is malformed!");
		Check<BadValueException>(!modelBin.empty(), "Cannot construct a model with empty data!");

		//Create implementation pointer
		impl = std::make_unique<Impl>();

		//Setup Assimp
		impl->importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);

		/*
		Load scene

		Post-processing flags applied are:
		JoinIdenticalVertices - Makes vertices used by more than one face the same (not stored that way by default)
		CalcTangentSpace - For meshes with normals (normal mapping will later be added and require this)
		Triangulate - Ensure all polygons are triangles with 3 indices
		SortByPType - Split meshes with 2+ primitive types into submeshes with 1 primitive type (and ignores lines and points due to configuration property above)
		*/
		impl->scene = impl->importer.ReadFileFromMemory(modelBin.data(), modelBin.size(), aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_SortByPType);

		//Check validity of scene
		Check<NonexistentValueException>(impl->scene != nullptr, std::string("Failed to load model data, Assimp provided error message: ") + impl->importer.GetErrorString());
		Check<NonexistentValueException>(impl->scene->HasMeshes(), "Model data does not contain any meshes!");

		//Calculate model orientation for axis correction
		int upAxis = 1, upAxisSign = 1;
		if(impl->scene->mMetaData) {
			impl->scene->mMetaData->Get<int>("UpAxis", upAxis);
			impl->scene->mMetaData->Get<int>("UpAxisSign", upAxisSign);
		}
		if(upAxis == 0) {
			impl->orient = (upAxisSign < 1 ? Impl::ModelOrientation::NegX : Impl::ModelOrientation::PosX);
		} else if(upAxis == 1) {
			impl->orient = (upAxisSign < 1 ? Impl::ModelOrientation::NegY : Impl::ModelOrientation::PosY);
		} else if(upAxis == 2) {
			impl->orient = (upAxisSign < 1 ? Impl::ModelOrientation::NegZ : Impl::ModelOrientation::PosZ);
		}

		//Populate mesh list
		std::vector<std::string> meshIDs;
		for(unsigned int i = 0; i < impl->scene->mNumMeshes; ++i) {
			aiMesh* mesh = impl->scene->mMeshes[i];
			std::string meshID = mesh->mName.length == 0 ? (std::string("Mesh") + std::to_string(i)) : std::string(mesh->mName.C_Str());
			impl->meshIndex.insert_or_assign(meshID, mesh);
			meshIDs.push_back(meshID);
		}

		//Texture scanning
		if(impl->scene->HasTextures()) {
			for(unsigned int i = 0; i < impl->scene->mNumTextures; ++i) {
				aiTexture* tex = impl->scene->mTextures[i];

				//Check if this is a known format
				std::string fmt = std::string(tex->achFormatHint).substr(0, 3);
				if(tex->mHeight == 0 && !(fmt.compare("web") == 0 || fmt.compare("jpg") == 0 || fmt.compare("x-t") == 0 || fmt.compare("png") == 0 || fmt.compare("tif") == 0)) continue;

				impl->textureIndex.insert_or_assign(tex->mFilename.length == 0 ? (std::string("Tex") + std::to_string(i)) : std::string(tex->mFilename.C_Str()), tex);
			}
		}
	}

	Model::~Model() {}

	const std::vector<std::string> Model::ListMeshes() {
		std::vector<std::string> accum;
		for(const auto& [id, _] : impl->meshIndex) {
			accum.push_back(id);
		}
		return accum;
	}

	const std::vector<std::string> Model::ListTextures() {
		std::vector<std::string> accum;
		for(const auto& [id, _] : impl->textureIndex) {
			accum.push_back(id);
		}
		return accum;
	}

	std::shared_ptr<Mesh> Model::GetMesh(const std::string& id) {
		Check<NonexistentValueException>(impl->meshIndex.contains(id), "Cannot retrieve nonexistent mesh from model!");

		//Get mesh info
		aiMesh* amesh = impl->meshIndex[id];
		bool hasTexCoord = amesh->HasTextureCoords(0);
		bool hasTanBitan = amesh->HasTangentsAndBitangents();
		bool hasNormals = amesh->HasNormals();

		//Create output buffers
		std::vector<Vertex> vertices;
		std::vector<glm::uvec3> indices;

		//Handle vertices
		for(unsigned int i = 0; i < amesh->mNumVertices; ++i) {
			//Get position
			aiVector3D vert = amesh->mVertices[i];
			glm::vec3 position = {vert.x, vert.y, vert.z};

			glm::vec2 texCoords = glm::vec2(0.0f);
			glm::vec3 tangent = glm::vec3(0.0f);
			glm::vec3 bitangent = glm::vec3(0.0f);
			glm::vec3 normal = glm::vec3(0.0f);

			//Texture coordinates
			if(hasTexCoord) {
				aiVector3D tc = amesh->mTextureCoords[0][i];
				texCoords = {tc.x, tc.y};
			}

			//Tangent and bitangent vectors
			if(hasTanBitan) {
				aiVector3D tan = amesh->mTangents[i];
				aiVector3D bitan = amesh->mBitangents[i];
				tangent = {tan.x, tan.y, tan.z};
				bitangent = {bitan.x, bitan.y, bitan.z};
			}

			//Normal vectors
			if(hasNormals) {
				aiVector3D norm = amesh->mNormals[i];
				normal = {norm.x, norm.y, norm.z};
			}

			//Apply axis correction
			switch(impl->orient) {
				case Impl::ModelOrientation::PosY:
					break;
				case Impl::ModelOrientation::NegY:
					position = glm::rotateZ(position, glm::radians(180.0f));
					break;
				case Impl::ModelOrientation::PosX:
					position = glm::rotateZ(position, glm::radians(-90.0f));
					break;
				case Impl::ModelOrientation::NegX:
					position = glm::rotateZ(position, glm::radians(90.0f));
					break;
				case Impl::ModelOrientation::PosZ:
					position = glm::rotateX(position, glm::radians(-90.0f));
					break;
				case Impl::ModelOrientation::NegZ:
					position = glm::rotateX(position, glm::radians(90.0f));
					break;
			}

			//Add vertex
			Vertex vertex {position, texCoords, tangent, bitangent, normal};
			vertices.push_back(vertex);
		}

		//Handle indices
		for(unsigned int i = 0; i < amesh->mNumFaces; ++i) {
			aiFace face = amesh->mFaces[i];
			indices.push_back({face.mIndices[0], face.mIndices[1], face.mIndices[2]});
		}

		//Construct and return mesh
		std::stringstream meshAddr("m:");
		meshAddr << address.substr(2) << "/" << id;
		std::shared_ptr<Mesh> mesh = Mesh::Create(std::move(vertices), std::move(indices), meshAddr.str());
		return mesh;
	}

	std::shared_ptr<Tex2D> Model::GetTexture(const std::string& id) {
		Check<NonexistentValueException>(impl->textureIndex.contains(id), "Cannot retrieve nonexistent mesh from texture!");

		//Get texture info
		aiTexture* tex = impl->textureIndex[id];
		std::string texType(tex->achFormatHint);

		//Unpack texels to bytes
		std::size_t texelCount = static_cast<std::size_t>(tex->mWidth) * std::clamp(tex->mHeight, (unsigned int)1, UINT32_MAX);
		std::vector<unsigned char> texelData(texelCount * 4);
		for(std::size_t i = 0; i < texelCount;) {
			std::size_t texel = i / 4;
			texelData[i++] = tex->pcData[texel].b;
			texelData[i++] = tex->pcData[texel].g;
			texelData[i++] = tex->pcData[texel].r;
			texelData[i++] = tex->pcData[texel].a;
		}

		//Get image data
		libcacaoimage::Image img;

		//Is this a compressed texture?
		if(tex->mHeight == 0) {
			//Check if we can handle the texture
			//Assimp truncates MIME type to 3 characters, so you get "web" for WebP, "x-t" for TGA (because MIME for that is "x-tga") and "tif" for TIFF
			std::string fmt = texType.substr(0, 3);
			Check<BadValueException>(fmt.compare("web") == 0 || fmt.compare("jpg") == 0 || fmt.compare("x-t") == 0 || fmt.compare("png") == 0 || fmt.compare("tif") == 0,
				"Cannot get a texture with an unsupported texture format!");

			//Convert texel data to std::vector<char> for streaming purposes
			//We also obliterate texelData as a part of this because we don't need two copies of the data
			//And we want to free up memory
			std::vector<char> charTexels(texelData.size());
			charTexels.assign(texelData.begin(), texelData.end());
			texelData.clear();
			texelData.shrink_to_fit();
			ibytestream texelstream(charTexels);

			//Try to decode texture data
			try {
				img = libcacaoimage::decode::DecodeGeneric(texelstream);
			} catch(const std::runtime_error& e) {
				//We re-throw the exception as a Cacao Engine exception to keep everything tidy
				Check<ExternalException>(false, std::string("While decoding model embedded texture data: \"") + e.what() + "\"");
			}
		} else {
			//Set image properties
			img.w = tex->mWidth;
			img.h = tex->mHeight;
			img.bitsPerChannel = 8;

			//Parse format hint
			std::unordered_map<char, uint8_t> channels;
			for(uint8_t i = 0; i < 4; ++i) {
				channels.insert_or_assign(texType[i], static_cast<unsigned char>(texType[i + 4] - '0'));
			}

			//Validate format
			uint8_t zeroedChannels = 0;
			for(const auto& [color, bits] : channels) {
				if(bits <= 0) ++zeroedChannels;
			}
			if(zeroedChannels != 0) CheckException(zeroedChannels % 2 != 0, "Invalid zeroed-channel layout for model embedded texture!");
			if(zeroedChannels == 1) CheckException(channels['a'] == 0, "Only the alpha channel may be zero for a model embedded texture layout with only one zeroed channel!");

			//Set layout in image object
			switch(zeroedChannels) {
				case 0: img.layout = libcacaoimage::Image::Layout::RGBA; break;
				case 1: img.layout = libcacaoimage::Image::Layout::RGB; break;
				case 3: img.layout = libcacaoimage::Image::Layout::Grayscale; break;
				default: CheckException(false, "Impossible channel layout for model embedded texture!");
			}

			//Move data to image object
			img.data.resize(texelData.size());
			if(zeroedChannels != 3) {
				//Calculate data shifts for data relocation
				//English: Figure out the new index of each channel per pixel to make it RGB(A)
				//This shift should be set to INT8_MAX to indicate that the channel should be skipped
				std::unordered_map<uint8_t, int8_t> shifts;
				std::string channelOrder = texType.substr(0, 4);
				std::string correctOrder("rgba");
				for(uint8_t i = 0; i < 4; ++i) {
					char channel = channelOrder[i];

					//Check if channel is zeroed and thus skipped
					if(channels[channel] == 0) {
						shifts[i] = INT8_MAX;
						continue;
					}

					//Calculate shift
					uint8_t correctPos = correctOrder.find(channel);
					shifts[i] = correctPos - i;
				}

				//Move the data
				std::size_t written = 0;
				for(std::size_t texel = 0; texel < texelCount; ++texel) {
					//Calculate base index of texel
					std::size_t base = texel * (4 - zeroedChannels);
					for(uint8_t channel = 0; channel < 4; ++channel) {
						//Skip?
						if(shifts[channel] == INT8_MAX) continue;

						//Color expansion to map to 8-bit color
						uint8_t writeByte = texelData[base + channel];
						uint8_t bits = channels[channelOrder[channel]];
						if(bits < 8) {
							writeByte = (writeByte & ((1 << bits) - 1)) << (8 - bits);
							writeByte |= writeByte >> bits;
						}

						//Write the color byte
						img.data[base + channel + shifts[channel]] = writeByte;
						++written;
					}
				}
				img.data.resize(written);
				img.data.shrink_to_fit();
			} else {
				//Grayscale data only needs expansion
				uint8_t grayBits = 0;
				for(const auto& [color, bits] : channels) {
					if(bits > 0) {
						grayBits = bits;
						break;
					}
				}
				for(std::size_t i = 0; i < texelData.size(); ++i) {
					uint8_t writeByte = texelData[i] << (8 - grayBits);
					writeByte |= (writeByte & ((1 << grayBits) - 1)) >> grayBits;
					img.data[i] = writeByte;
				}
			}
		}

		//Okay so now we finally have the texture data in the correct format
		//Now we can make the Tex2D
		std::stringstream texAddr("m:");
		texAddr << address.substr(2) << "%" << id;
		std::shared_ptr<Tex2D> t2d = Tex2D::Create(std::move(img), texAddr.str());
		return t2d;
	}
}