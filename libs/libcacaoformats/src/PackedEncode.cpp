#include "libcacaoformats/libcacaoformats.hpp"

#include "CheckException.hpp"

#include <sstream>
#include <utility>
#include <iostream>

#define LIBARCHIVE_STATIC
#include "yaml-cpp/yaml.h"
#include "archive.h"
#include "archive_entry.h"

namespace libcacaoformats {
	PackedContainer PackedEncoder::EncodeCubemap(const std::array<ImageBuffer, 6>& cubemap) {
		//Validate input
		uint64_t sizes[] = {0, 0, 0, 0, 0, 0};
		uint8_t i = 0;
		for(const ImageBuffer& buf : cubemap) {
			CheckException(buf.channelCount > 0 && buf.size.x > 0 && buf.size.y > 0, "Cubemap face for packed encoding has invalid dimensions or channel count!");
			CheckException(!buf.data.empty(), "Cubemap face for packed encoding has empty data buffer!");
			sizes[i++] = (uint64_t)buf.size.x * (uint64_t)buf.size.y * (uint64_t)buf.channelCount;
		}

		//Create output container
		std::vector<char> outBuffer;
		obytestream out(outBuffer);

		//Write face sizes
		out.write(reinterpret_cast<char*>(&sizes[0]), sizeof(uint64_t));
		out.write(reinterpret_cast<char*>(&sizes[1]), sizeof(uint64_t));
		out.write(reinterpret_cast<char*>(&sizes[2]), sizeof(uint64_t));
		out.write(reinterpret_cast<char*>(&sizes[3]), sizeof(uint64_t));
		out.write(reinterpret_cast<char*>(&sizes[4]), sizeof(uint64_t));
		out.write(reinterpret_cast<char*>(&sizes[5]), sizeof(uint64_t));

		//Encode face data and write it out
		EncodeImage(cubemap[0], out);
		EncodeImage(cubemap[1], out);
		EncodeImage(cubemap[2], out);
		EncodeImage(cubemap[3], out);
		EncodeImage(cubemap[4], out);
		EncodeImage(cubemap[5], out);

		//Create and return packed container
		return PackedContainer(FormatCode::Cubemap, 1, std::move(outBuffer));
	}

	PackedContainer PackedEncoder::EncodeShader(const Shader& shader) {
		//Validate input
		CheckException(shader.vertexSPV.size() > 0, "Shader for packed encoding has zero-size vertex SPIR-V!");
		CheckException(shader.fragmentSPV.size() > 0, "Shader for packed encoding has zero-size fragment SPIR-V!");

		//Create output container
		std::vector<char> outBuffer;
		obytestream out(outBuffer);

		//Write face sizes
		uint32_t vertexSize = shader.vertexSPV.size() * sizeof(uint32_t);
		uint32_t fragmentSize = shader.vertexSPV.size() * sizeof(uint32_t);
		out.write(reinterpret_cast<char*>(&vertexSize), sizeof(uint32_t));
		out.write(reinterpret_cast<char*>(&fragmentSize), sizeof(uint32_t));

		//Write SPIR-V
		out.write(reinterpret_cast<const char*>(shader.vertexSPV.data()), vertexSize);
		out.write(reinterpret_cast<const char*>(shader.fragmentSPV.data()), fragmentSize);

		//Create and return packed container
		return PackedContainer(FormatCode::Shader, 1, std::move(outBuffer));
	}

	PackedContainer PackedEncoder::EncodeMaterial(const Material& mat) {
		//Validate input
		CheckException(mat.shader.size() > 0, "Material for packed encoding has zero-length shader address string!");
		CheckException(mat.shader.size() <= UINT16_MAX, "Material for packed encoding has too long shader address string!");
		CheckException(mat.keys.size() < 256, "Material for packed encoding has too many keys (>255)!");

		//Create output container
		std::vector<char> outBuffer;
		obytestream out(outBuffer);

		//Write shader address string
		uint16_t saLen = (uint16_t)mat.shader.size();
		out.write(reinterpret_cast<char*>(&saLen), 2);
		out << mat.shader;

		//Write key count
		uint8_t keyCount = (uint8_t)mat.keys.size();
		out.write(reinterpret_cast<char*>(&keyCount), 1);

		for(const auto& key : mat.keys) {
			//Write key name string
			CheckException(key.first.size() > 0 && key.first.size() <= UINT8_MAX, "Material key for packed encoding has out-of-range name string length!");
			uint8_t knLen = (uint8_t)key.first.size();
			out.write(reinterpret_cast<char*>(&knLen), 1);
			out << key.first;

			//Write data and type info
			uint8_t typeInfo;
			std::vector<char> dataBuffer;
			obytestream data(dataBuffer);
			try {
				switch(key.second.index()) {
					case 0: {
						typeInfo = 0b00000000;
						int value = std::get<0>(key.second);
						data.write(reinterpret_cast<char*>(&value), sizeof(value));
						break;
					}
					case 1: {
						typeInfo = 0b00000001;
						unsigned int value = std::get<1>(key.second);
						data.write(reinterpret_cast<char*>(&value), sizeof(value));
						break;
					}
					case 2: {
						typeInfo = 0b00000010;
						float value = std::get<2>(key.second);
						data.write(reinterpret_cast<char*>(&value), sizeof(value));
						break;
					}
					case 3: {
						typeInfo = 0b00010000;
						Vec2<int> vec = std::get<3>(key.second);
						int value[] = {vec.x, vec.y};
						data.write(reinterpret_cast<char*>(&value), sizeof(value));
						break;
					}
					case 4: {
						typeInfo = 0b00100000;
						Vec3<int> vec = std::get<4>(key.second);
						int value[] = {vec.x, vec.y, vec.z};
						data.write(reinterpret_cast<char*>(&value), sizeof(value));
						break;
					}
					case 5: {
						typeInfo = 0b00110000;
						Vec4<int> vec = std::get<5>(key.second);
						int value[] = {vec.x, vec.y, vec.z, vec.w};
						out.write(reinterpret_cast<char*>(&value), sizeof(value));
						break;
					}
					case 6: {
						typeInfo = 0b00010001;
						Vec2<unsigned int> vec = std::get<6>(key.second);
						unsigned int value[] = {vec.x, vec.y};
						out.write(reinterpret_cast<char*>(&value), sizeof(value));
						break;
					}
					case 7: {
						typeInfo = 0b00100001;
						Vec3<unsigned int> vec = std::get<7>(key.second);
						unsigned int value[] = {vec.x, vec.y, vec.z};
						data.write(reinterpret_cast<char*>(&value), sizeof(value));
						break;
					}
					case 8: {
						typeInfo = 0b00110001;
						Vec4<unsigned int> vec = std::get<8>(key.second);
						unsigned int value[] = {vec.x, vec.y, vec.z, vec.w};
						data.write(reinterpret_cast<char*>(&value), sizeof(value));
						break;
					}
					case 9: {
						typeInfo = 0b00010010;
						Vec2<float> vec = std::get<9>(key.second);
						float value[] = {vec.x, vec.y};
						data.write(reinterpret_cast<char*>(&value), sizeof(value));
						break;
					}
					case 10: {
						typeInfo = 0b00100010;
						Vec3<float> vec = std::get<10>(key.second);
						float value[] = {vec.x, vec.y, vec.z};
						data.write(reinterpret_cast<char*>(&value), sizeof(value));
						break;
					}
					case 11: {
						typeInfo = 0b00110010;
						Vec4<float> vec = std::get<11>(key.second);
						float value[] = {vec.x, vec.y, vec.z, vec.w};
						data.write(reinterpret_cast<char*>(&value), sizeof(value));
						break;
					}
					case 12: {
						typeInfo = 0b00010110;
						Matrix<float, 2, 2> m = std::get<12>(key.second);
						float value[] = {
							m.data[0][0],
							m.data[1][0],
							m.data[0][1],
							m.data[1][1]};
						data.write(reinterpret_cast<char*>(&value), sizeof(value));
						break;
					}
					case 13: {
						typeInfo = 0b00100110;
						Matrix<float, 2, 3> m = std::get<13>(key.second);
						float value[] = {
							m.data[0][0],
							m.data[1][0],
							m.data[2][0],
							m.data[0][1],
							m.data[1][1],
							m.data[2][1]};
						data.write(reinterpret_cast<char*>(&value), sizeof(value));
						break;
					}
					case 14: {
						typeInfo = 0b00110110;
						Matrix<float, 2, 4> m = std::get<14>(key.second);
						float value[] = {
							m.data[0][0],
							m.data[1][0],
							m.data[2][0],
							m.data[3][0],
							m.data[0][1],
							m.data[1][1],
							m.data[2][1],
							m.data[3][1]};
						data.write(reinterpret_cast<char*>(&value), sizeof(value));
						break;
					}
					case 15: {
						typeInfo = 0b00011010;
						Matrix<float, 3, 2> m = std::get<15>(key.second);
						float value[] = {
							m.data[0][0],
							m.data[1][0],
							m.data[0][1],
							m.data[1][1],
							m.data[0][2],
							m.data[1][2]};
						data.write(reinterpret_cast<char*>(&value), sizeof(value));
						break;
					}
					case 16: {
						typeInfo = 0b00101010;
						Matrix<float, 3, 3> m = std::get<16>(key.second);
						float value[] = {
							m.data[0][0],
							m.data[1][0],
							m.data[2][0],
							m.data[0][1],
							m.data[1][1],
							m.data[2][1],
							m.data[0][2],
							m.data[1][2],
							m.data[2][2]};
						data.write(reinterpret_cast<char*>(&value), sizeof(value));
						break;
					}
					case 17: {
						typeInfo = 0b00111010;
						Matrix<float, 3, 4> m = std::get<17>(key.second);
						float value[] = {
							m.data[0][0],
							m.data[1][0],
							m.data[2][0],
							m.data[3][0],
							m.data[0][1],
							m.data[1][1],
							m.data[2][1],
							m.data[3][1],
							m.data[0][2],
							m.data[1][2],
							m.data[2][2],
							m.data[3][2]};
						data.write(reinterpret_cast<char*>(&value), sizeof(value));
						break;
					}
					case 18: {
						typeInfo = 0b00011110;
						Matrix<float, 4, 2> m = std::get<18>(key.second);
						float value[] = {
							m.data[0][0],
							m.data[1][0],
							m.data[0][1],
							m.data[1][1],
							m.data[0][2],
							m.data[1][2],
							m.data[0][3],
							m.data[1][3]};
						data.write(reinterpret_cast<char*>(&value), sizeof(value));
						break;
					}
					case 19: {
						typeInfo = 0b00101110;
						Matrix<float, 4, 3> m = std::get<19>(key.second);
						float value[] = {
							m.data[0][0],
							m.data[1][0],
							m.data[2][0],
							m.data[0][1],
							m.data[1][1],
							m.data[2][1],
							m.data[0][2],
							m.data[1][2],
							m.data[2][2],
							m.data[0][3],
							m.data[1][3],
							m.data[2][3]};
						data.write(reinterpret_cast<char*>(&value), sizeof(value));
						break;
					}
					case 20: {
						typeInfo = 0b00111110;
						Matrix<float, 4, 4> m = std::get<20>(key.second);
						float value[] = {
							m.data[0][0],
							m.data[1][0],
							m.data[2][0],
							m.data[3][0],
							m.data[0][1],
							m.data[1][1],
							m.data[2][1],
							m.data[3][1],
							m.data[0][2],
							m.data[1][2],
							m.data[2][2],
							m.data[3][2],
							m.data[0][3],
							m.data[1][3],
							m.data[2][3],
							m.data[3][3]};
						data.write(reinterpret_cast<char*>(&value), sizeof(value));
						break;
					}
					case 21: {
						typeInfo = 0b00000011;
						Material::TextureRef value = std::get<21>(key.second);
						if(value.isCubemap) typeInfo |= 0b01000000;
						CheckException(value.path.size() > 0 && value.path.size() <= UINT16_MAX, "Material key for packed encoding has out-of-range texture reference string length!");
						uint16_t taLen = (uint16_t)value.path.size();
						data.write(reinterpret_cast<char*>(&taLen), 2);
						data << value.path;
						break;
					}
					default: CheckException(false, "Material for packed encoding has key of invalid type!");
				}
			} catch(const std::bad_variant_access&) {
				CheckException(false, "Key value type does not match listed type in variant! (somehow)");
			}

			//Write type info
			out.write(reinterpret_cast<char*>(&typeInfo), 1);

			//Write data
			out.write(dataBuffer.data(), dataBuffer.size() * sizeof(char));

			//Write separator
			out.write("\0\0", 2);
		}

		//Create and return packed container
		return PackedContainer(FormatCode::Material, 1, std::move(outBuffer));
	}

	PackedContainer PackedEncoder::EncodeWorld(const World& world) {
		//Create output container
		std::vector<char> outBuffer;
		obytestream out(outBuffer);

		//Write skybox address string
		uint16_t skyboxLen = (uint16_t)world.skyboxRef.size();
		out.write(reinterpret_cast<char*>(&skyboxLen), 2);
		if(skyboxLen > 0) out << world.skyboxRef;

		//Write initial camera data
		float initialCamData[] = {world.initialCamPos.x, world.initialCamPos.y, world.initialCamPos.z, world.initialCamRot.x, world.initialCamRot.y, world.initialCamRot.z};
		out.write(reinterpret_cast<char*>(&initialCamData), sizeof(initialCamData));

		//Write entity count
		uint64_t entityCount = world.entities.size();
		out.write(reinterpret_cast<char*>(&entityCount), 8);

		const auto parseGUID = [](const std::string& input) {
			std::string hexNum;
			for(char c : input) {
				if(c == '-') continue;
				if(c < 48 || c > 102 || (c > 57 && c < 97)) {
					std::stringstream ss;
					ss << "Invalid GUID character \"" << c << "\" found during packed encoding in world entity!";
					CheckException(false, ss.str());
				}
				hexNum += c;
			}
			CheckException(hexNum.size() == 32, "GUID string for world entity contains too many or two few digits!");
			std::string upperStr = hexNum.substr(0, 16);
			std::string lowerStr = hexNum.substr(16, 16);

			//Hex conversion trickery
			std::stringstream converter;
			uint64_t upper;
			converter << upperStr;
			converter >> upper;
			converter.str("");
			uint64_t lower;
			converter << lowerStr;
			converter >> lower;

			//Output the numbers
			return std::make_pair<uint64_t, uint64_t>(std::move(upper), std::move(lower));
		};

		//Write entities
		for(const World::Entity& entity : world.entities) {
			//Write entity GUID
			auto [guidUpper, guidLower] = parseGUID(entity.guid);
			uint64_t guid[] = {guidUpper, guidLower};
			out.write(reinterpret_cast<char*>(&guid), sizeof(guid));

			//Write entity parent GUID
			auto [parentUpper, parentLower] = parseGUID(entity.guid);
			uint64_t parentGUID[] = {parentUpper, parentLower};
			out.write(reinterpret_cast<char*>(&parentGUID), sizeof(parentGUID));

			//Write entity name string
			CheckException(entity.name.size() > 0 && entity.name.size() <= UINT16_MAX, "World entity for packed encoding has out-of-range name string length!");
			uint16_t enLen = (uint8_t)entity.name.size();
			out.write(reinterpret_cast<char*>(&enLen), 2);
			out << entity.name;

			//Write initial transform data
			float transformData[] = {entity.initialPos.x, entity.initialPos.y, entity.initialPos.z, entity.initialRot.x, entity.initialRot.y, entity.initialRot.z, entity.initialScale.x, entity.initialScale.y, entity.initialScale.z};
			out.write(reinterpret_cast<char*>(transformData), sizeof(transformData));

			//Write component count
			CheckException(entity.components.size() > 0 && entity.components.size() <= UINT8_MAX, "World entity for packed encoding has invalid component count!");
			uint8_t compCount = (uint8_t)entity.components.size();
			out.write(reinterpret_cast<char*>(&compCount), 1);

			//Write components
			for(const World::Component& component : entity.components) {
				//Write type ID string
				CheckException(component.typeID.size() > 0 && component.typeID.size() <= UINT16_MAX, "World entity component for packed encoding has out-of-range type ID string length!");
				uint16_t tiLen = (uint16_t)component.typeID.size();
				out.write(reinterpret_cast<char*>(&tiLen), 2);
				out << component.typeID;

				//Write reflection data
				CheckException(component.data.size() > 0 && component.data.size() <= UINT16_MAX, "World entity component for packed encoding has out-of-range reflection data size!");
				uint32_t rdLen = (uint32_t)component.data.size();
				out.write(reinterpret_cast<char*>(&rdLen), 2);
				out.write(reinterpret_cast<const char*>(component.data.data()), component.data.size() * sizeof(unsigned char));

				//Write component separator
				out.write("\0\0", 2);
			}

			//Write entity separator
			out << "%e";
		}

		//Create and return packed container
		return PackedContainer(FormatCode::World, 1, std::move(outBuffer));
	}

	PackedContainer PackedEncoder::EncodeAssetPack(const std::map<std::string, PackedAsset>& pack) {
		//Validate inputs
		CheckException(pack.size() > 0, "Cannot encode asset pack with no assets!");

		//Create output container with a default size (we might change this later)
		std::vector<char> outBuffer(1042 ^ 2);
		std::size_t used;

		//Create metadata file emitter
		YAML::Emitter yml;
		yml << YAML::BeginSeq;

		//Configure archive object
		archive* pak = archive_write_new();
		CheckException(pak, "Unable to create asset pack archive object!");
		archive_write_set_format_pax_restricted(pak);
		CheckException(archive_write_open_memory(pak, outBuffer.data(), outBuffer.capacity() * sizeof(char), &used) == ARCHIVE_OK, "Failed to open archive object for asset pack encoding!");

		//Write files
		archive_entry* entry = archive_entry_new();
		CheckException(entry, "Unable to create asset pack entry object");
		for(const auto& asset : pack) {
			//Reset entry object
			entry = archive_entry_clear(entry);

			//Fill out entry data
			archive_entry_set_pathname_utf8(entry, asset.first.c_str());
			archive_entry_set_size(entry, asset.second.buffer.size() * sizeof(unsigned char));
			archive_entry_set_filetype(entry, AE_IFREG);
			archive_entry_set_perm(entry, 0644);

			//Write entry header
			int stat = archive_write_header(pak, entry);
			if(stat != ARCHIVE_OK) {
				if(used > outBuffer.capacity() * sizeof(char)) {
					//Resize the buffer and try again
					outBuffer.reserve(outBuffer.size() * 1.5);
					CheckException(archive_write_header(pak, entry) == ARCHIVE_OK, "Failed to write archive entry header for asset pack encoding!");
				} else {
					CheckException(false, "Failed to write archive entry header for asset pack encoding!");
				}
			}

			//Write file data
			archive_write_data(pak, asset.second.buffer.data(), asset.second.buffer.size() * sizeof(unsigned char));

			//Output metadata entry
			yml << YAML::BeginMap << YAML::Key << "asset" << YAML::Value << asset.first << YAML::Key << "type" << YAML::Value;
			switch(asset.second.kind) {
				case PackedAsset::Kind::Shader:
					yml << 0;
					break;
				case PackedAsset::Kind::Tex2D:
					yml << 1;
					break;
				case PackedAsset::Kind::Cubemap:
					yml << 2;
					break;
				case PackedAsset::Kind::Sound:
					yml << 3;
					break;
				case PackedAsset::Kind::Material:
					yml << 4;
					break;
				case PackedAsset::Kind::Font:
					yml << 5;
					break;
				default:
					yml << -1;
					break;
			}
			yml << YAML::EndMap;
		}
		yml << YAML::EndSeq;

		//Now we do the whole writing stuff again for the metadata file
		{
			//Get data
			std::string meta(yml.c_str());

			//Reset entry object
			entry = archive_entry_clear(entry);

			//Fill out entry data
			archive_entry_set_pathname_utf8(entry, "__$CacaoMeta0");
			archive_entry_set_size(entry, meta.size() * sizeof(char));
			archive_entry_set_filetype(entry, AE_IFREG);
			archive_entry_set_perm(entry, 0644);

			//Write entry header
			int stat = archive_write_header(pak, entry);
			if(stat != ARCHIVE_OK) {
				if(used > outBuffer.capacity() * sizeof(char)) {
					//Resize the buffer and try again
					outBuffer.reserve(outBuffer.size() * 1.5);
					CheckException(archive_write_header(pak, entry) == ARCHIVE_OK, "Failed to write metadata file entry header for asset pack encoding!");
				} else {
					CheckException(false, "Failed to write metadata file entry header for asset pack encoding!");
				}
			}

			//Write file data
			archive_write_data(pak, meta.c_str(), meta.size() * sizeof(unsigned char));
		}

		//Close and free archive object
		CheckException(archive_write_close(pak) == ARCHIVE_OK, "Failed to close asset pack archive object!");
		CheckException(archive_write_free(pak) == ARCHIVE_OK, "Failed to free asset pack archive object!");

		//Create and return packed container
		return PackedContainer(FormatCode::AssetPack, 1, std::move(outBuffer));
	}
}