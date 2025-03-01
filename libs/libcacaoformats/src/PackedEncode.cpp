#include "libcacaoformats/libcacaoformats.hpp"

#include "CheckException.hpp"

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
			if(--keyCount == 0) out << "\0\0";
		}

		//Create and return packed container
		return PackedContainer(FormatCode::Material, 1, std::move(outBuffer));
	}
}