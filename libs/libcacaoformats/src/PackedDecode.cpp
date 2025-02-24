#include "libcacaoformats/libcacaoformats.hpp"

#include "CheckException.hpp"

#include <cstdint>
#include <cstring>

namespace libcacaoformats {
	std::array<ImageBuffer, 6> PackedDecoder::DecodeCubemap(const PackedContainer& container) {
		CheckException(container.format == FormatCode::Cubemap, "Packed container provided for cubemap decoding is not a cubemap!");
		CheckException(container.payload.size() > 28, "Cubemap packed container is too small to contain face size data!");

		//Get buffer sizes
		uint32_t pxs, nxs, pys, nys, pzs, nzs;
		std::memcpy(&pxs, container.payload.data(), 4);
		std::memcpy(&nxs, container.payload.data() + 4, 4);
		std::memcpy(&pys, container.payload.data() + 8, 4);
		std::memcpy(&nys, container.payload.data() + 12, 4);
		std::memcpy(&pzs, container.payload.data() + 16, 4);
		std::memcpy(&nzs, container.payload.data() + 24, 4);

		//Extract encoded face buffers
		std::vector<char> pxFB(pxs), nxFB(nxs), pyFB(pys), nyFB(nys), pzFB(pzs), nzFB(nzs);
		std::size_t offsetCounter = 28;
		CheckException(container.payload.size() > (28 + pxs + nxs + pys + nys + pzs + nzs), "Cubemap packed container is too small to contain face data of given sizes!");
		std::memcpy(pxFB.data(), container.payload.data() + offsetCounter, pxs);
		offsetCounter += pxs;
		std::memcpy(nxFB.data(), container.payload.data() + offsetCounter, nxs);
		offsetCounter += nxs;
		std::memcpy(pyFB.data(), container.payload.data() + offsetCounter, pys);
		offsetCounter += pys;
		std::memcpy(nyFB.data(), container.payload.data() + offsetCounter, nys);
		offsetCounter += nys;
		std::memcpy(pzFB.data(), container.payload.data() + offsetCounter, pzs);
		offsetCounter += pzs;
		std::memcpy(nzFB.data(), container.payload.data() + offsetCounter, nzs);
		offsetCounter += nzs;

		//Decode face buffers
		std::array<ImageBuffer, 6> out {};
		ibytestream pxStream(pxFB);
		out[0] = DecodeImage(pxStream);
		ibytestream nxStream(nxFB);
		out[1] = DecodeImage(nxStream);
		ibytestream pyStream(pyFB);
		out[2] = DecodeImage(pyStream);
		ibytestream nyStream(nyFB);
		out[3] = DecodeImage(nyStream);
		ibytestream pzStream(pzFB);
		out[4] = DecodeImage(pzStream);
		ibytestream nzStream(nzFB);
		out[5] = DecodeImage(nzStream);

		//Return result
		return out;
	}

	Shader PackedDecoder::DecodeShader(const PackedContainer& container) {
		CheckException(container.format == FormatCode::Shader, "Packed container provided for shader decoding is not a shader!");
		CheckException(container.payload.size() > 8, "Shader packed container is too small to contain code size data!");

		//Get buffer sizes
		uint32_t vscsz, pscsz;
		std::memcpy(&vscsz, container.payload.data(), 4);
		std::memcpy(&pscsz, container.payload.data() + 4, 4);

		//Extract code buffers
		Shader shader {};
		shader.vertexSPV.reserve(vscsz);
		shader.fragmentSPV.reserve(pscsz);
		std::memcpy(shader.vertexSPV.data(), container.payload.data() + 8, vscsz);
		std::memcpy(shader.fragmentSPV.data(), container.payload.data() + 8 + vscsz, pscsz);

		//Return result
		return shader;
	}

	Material PackedDecoder::DecodeMaterial(const PackedContainer& container) {
		CheckException(container.format == FormatCode::Material, "Packed container provided for material decoding is not a material!");
		CheckException(container.payload.size() > 2, "Material packed container is too small to contain shader address string size data!");

		Material out {};

		//Get shader address string
		uint16_t saLen = 0;
		std::memcpy(&saLen, container.payload.data(), 2);
		CheckException(saLen > 0, "Material packed container has zero-length shader address string");
		CheckException(container.payload.size() > 2 + saLen, "Material packed container is too small to contain shader address string!");
		out.shader = std::string("\0", saLen);
		std::memcpy(out.shader.data(), container.payload.data() + 2, saLen);

		//Get material keys count
		uint8_t numKeys = 0;
		std::memcpy(&numKeys, container.payload.data() + 2 + saLen, 1);

		//If we don't need to process keys, return value now
		if(numKeys == 0) return out;

		//Process keys
		std::size_t offsetCounter = saLen + 3;
		for(uint8_t i = 0; i < numKeys; i++) {
			//Get type info
			CheckException(container.payload.size() > offsetCounter + 1, "Material packed container is too small to contain provided key count!");
			uint8_t typeInfo = 0;
			std::memcpy(&typeInfo, container.payload.data() + offsetCounter++, 1);

			//Get key name
			CheckException(container.payload.size() > offsetCounter + 1, "Material packed container is too small to contain key name length!");
			uint32_t keyNameLen = 0;
			std::memcpy(&keyNameLen, container.payload.data() + offsetCounter++, 1);
			CheckException(container.payload.size() > offsetCounter + 1, "Material packed container is too small to contain key name string of provided length!");
			std::string keyName("\0", keyNameLen);
			std::memcpy(keyName.data(), container.payload.data() + offsetCounter, keyNameLen);
			offsetCounter += keyNameLen;

			//Extract info and check size constraints
			Vec2<uint8_t> size;
			size.x = (typeInfo & 0b00001100);
			size.y = (typeInfo & 0b00110000);
			uint8_t baseType = (typeInfo & 0b00000011);
			CheckException(baseType == 3 && (size.x > 1 || size.y > 1), "Material packed container key has invalid size for texture type (must be 1x1)!");
			CheckException(baseType != 2 && size.y > 1, "Material packed container key has invalid size for non-float type (y must be 1)!");
			CheckException(size.x == 1 && size.y != 1, "Material packed container key has invalid size (if x is 1, must be 1x1)");
			uint8_t dims = (4 * size.y) - (4 - size.x);

			//Load data
			if(baseType != 3) CheckException(container.payload.size() > offsetCounter + (size.x * size.y * 4), "Material packed container key is too small to contain value of provided type!");
			switch(baseType) {
				case 0:
					switch(dims) {
						case 1: {
							int32_t val = 0;
							std::memcpy(&val, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							out.values.insert_or_assign(keyName, val);
							break;
						}
						case 2: {
							int32_t x = 0, y = 0;
							std::memcpy(&x, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&y, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							out.values.insert_or_assign(keyName, Vec2<int> {.x = x, .y = y});
							break;
						}
						case 3: {
							int32_t x = 0, y = 0, z = 0;
							std::memcpy(&x, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&y, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&z, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							out.values.insert_or_assign(keyName, Vec3<int> {.x = x, .y = y, .z = z});
							break;
						}
						case 4: {
							int32_t x = 0, y = 0, z = 0, w = 0;
							std::memcpy(&x, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&y, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&z, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&w, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							out.values.insert_or_assign(keyName, Vec4<int> {.x = x, .y = y, .z = z, .w = w});
							break;
						}
						default:
							break;
					}
					break;
				case 1:
					switch(dims) {
						case 1: {
							uint32_t val = 0;
							std::memcpy(&val, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							out.values.insert_or_assign(keyName, val);
							break;
						}
						case 2: {
							uint32_t x = 0, y = 0;
							std::memcpy(&x, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&y, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							out.values.insert_or_assign(keyName, Vec2<unsigned int> {.x = x, .y = y});
							break;
						}
						case 3: {
							uint32_t x = 0, y = 0, z = 0;
							std::memcpy(&x, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&y, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&z, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							out.values.insert_or_assign(keyName, Vec3<unsigned int> {.x = x, .y = y, .z = z});
							break;
						}
						case 4: {
							uint32_t x = 0, y = 0, z = 0, w = 0;
							std::memcpy(&x, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&y, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&z, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&w, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							out.values.insert_or_assign(keyName, Vec4<unsigned int> {.x = x, .y = y, .z = z, .w = w});
							break;
						}
						default:
							break;
					}
					break;
				case 2:
					switch(dims) {
						case 1: {
							float val = 0;
							std::memcpy(&val, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							out.values.insert_or_assign(keyName, val);
							break;
						}
						case 2: {
							float x = 0, y = 0;
							std::memcpy(&x, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&y, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							out.values.insert_or_assign(keyName, Vec2<float> {.x = x, .y = y});
							break;
						}
						case 3: {
							float x = 0, y = 0, z = 0;
							std::memcpy(&x, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&y, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&z, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							out.values.insert_or_assign(keyName, Vec3<float> {.x = x, .y = y, .z = z});
							break;
						}
						case 4: {
							float x = 0, y = 0, z = 0, w = 0;
							std::memcpy(&x, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&y, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&z, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&w, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							out.values.insert_or_assign(keyName, Vec4<float> {.x = x, .y = y, .z = z, .w = w});
							break;
						}
						case 6: {
							Matrix<float, 2, 2> m;
							std::memcpy(&m.data[0][0], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[1][0], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[0][1], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[1][1], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							out.values.insert_or_assign(keyName, m);
							break;
						}
						case 7: {
							Matrix<float, 2, 3> m;
							std::memcpy(&m.data[0][0], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[1][0], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[2][0], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[0][1], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[1][1], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[2][1], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							out.values.insert_or_assign(keyName, m);
							break;
						}
						case 8: {
							Matrix<float, 2, 4> m;
							std::memcpy(&m.data[0][0], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[1][0], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[2][0], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[3][0], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[0][1], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[1][1], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[2][1], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[3][1], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							out.values.insert_or_assign(keyName, m);
							break;
						}
						case 10: {
							Matrix<float, 3, 2> m;
							std::memcpy(&m.data[0][0], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[1][0], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[0][1], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[1][1], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[0][2], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[1][2], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							out.values.insert_or_assign(keyName, m);
							break;
						}
						case 11: {
							Matrix<float, 3, 3> m;
							std::memcpy(&m.data[0][0], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[1][0], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[2][0], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[0][1], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[1][1], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[2][1], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[0][2], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[1][2], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[2][2], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							out.values.insert_or_assign(keyName, m);
							break;
						}
						case 12: {
							Matrix<float, 3, 4> m;
							std::memcpy(&m.data[0][0], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[1][0], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[2][0], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[3][0], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[0][1], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[1][1], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[2][1], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[3][1], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[0][2], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[1][2], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[2][2], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[3][2], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							out.values.insert_or_assign(keyName, m);
							break;
						}
						case 14: {
							Matrix<float, 4, 2> m;
							std::memcpy(&m.data[0][0], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[1][0], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[0][1], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[1][1], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[0][2], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[1][2], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[0][3], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[1][3], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							out.values.insert_or_assign(keyName, m);
							break;
						}
						case 15: {
							Matrix<float, 4, 3> m;
							std::memcpy(&m.data[0][0], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[1][0], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[2][0], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[0][1], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[1][1], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[2][1], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[0][2], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[1][2], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[2][2], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[0][3], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[1][3], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[2][3], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							out.values.insert_or_assign(keyName, m);
							break;
						}
						case 16: {
							Matrix<float, 4, 4> m;
							std::memcpy(&m.data[0][0], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[1][0], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[2][0], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[3][0], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[0][1], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[1][1], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[2][1], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[3][1], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[0][2], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[1][2], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[2][2], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[3][2], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[0][3], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[1][3], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[2][3], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&m.data[3][3], container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							out.values.insert_or_assign(keyName, m);
							break;
						}
						default:
							break;
					}
					break;
				case 3: {
					Material::TextureRef ref {};
					CheckException(container.payload.size() > offsetCounter + 1, "Material packed container key is too small to contain texture reference string length!");
					uint32_t texLen = 0;
					std::memcpy(&texLen, container.payload.data() + offsetCounter++, 1);
					CheckException(container.payload.size() > offsetCounter + 1, "Material packed container key is too small to contain texture reference string of provided length!");
					ref.path = std::string("\0", texLen);
					std::memcpy(ref.path.data(), container.payload.data() + offsetCounter, texLen);
					offsetCounter += texLen;
					ref.isCubemap = (typeInfo & 0b01000000) > 0;
					out.values.insert_or_assign(keyName, ref);
					break;
				}
			}

			//Advance by two extra bytes since we put two null bytes as a separator between keys
			offsetCounter += 2;
		}
	}

	World PackedDecoder::DecodeWorld(const PackedContainer& container) {
		CheckException(container.format == FormatCode::World, "Packed container provided for world decoding is not a world!");
		CheckException(container.payload.size() > 2, "World packed container is too small to contain skybox address string size data!");

		World out {};
		std::size_t advance = 0;

		//Get skybox address string
		uint16_t saLen = 0;
		std::memcpy(&saLen, container.payload.data(), 2);
		advance += 2;
		CheckException(saLen >= 0, "World packed container has negative-length skybox address string");
		if(saLen == 0) {
			out.skyboxRef = "";
		} else {
			CheckException(container.payload.size() > 2 + saLen, "World packed container is too small to contain skybox address string!");
			out.skyboxRef = std::string("\0", saLen);
			std::memcpy(out.skyboxRef.data(), container.payload.data() + 2, saLen);
			advance += saLen;
		}

		//Get initial camera data
		CheckException(container.payload.size() > advance + 24, "World packed container is too small to contain initial camera data!");
		std::memcpy(&out.initialCamPos.x, container.payload.data() + advance, 4);
		advance += 4;
		std::memcpy(&out.initialCamPos.y, container.payload.data() + advance, 4);
		advance += 4;
		std::memcpy(&out.initialCamPos.z, container.payload.data() + advance, 4);
		advance += 4;
		std::memcpy(&out.initialCamRot.x, container.payload.data() + advance, 4);
		advance += 4;
		std::memcpy(&out.initialCamRot.y, container.payload.data() + advance, 4);
		advance += 4;
		std::memcpy(&out.initialCamRot.z, container.payload.data() + advance, 4);
		advance += 4;

		//Get entity count
		CheckException(container.payload.size() > advance + 8, "World packed container is too small to contain entity data!");
		uint64_t entityCount = 0;
		std::memcpy(&entityCount, container.payload.data() + advance, 8);
		advance += 8;

		//If we don't need to process entities, return value now
		if(entityCount == 0) return out;

		//TODO: Add entity decoding
	}
}