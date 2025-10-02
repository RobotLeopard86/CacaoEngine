#include "libcacaoformats.hpp"

#include "libcacaocommon.hpp"
#include "YAMLValidate.hpp"

#include <cstdint>
#include <cstring>
#include <filesystem>

#define LIBARCHIVE_STATIC
#include "archive.h"
#include "archive_entry.h"

namespace libcacaoformats {
	std::array<libcacaoimage::Image, 6> PackedDecoder::DecodeCubemap(const PackedContainer& container) {
		CheckException(container.format == PackedFormat::Cubemap, "Packed container provided for cubemap decoding is not a cubemap!");
		CheckException(container.payload.size() > 48, "Cubemap packed container is too small to contain face size data!");

		//Get buffer sizes
		uint64_t pxs, nxs, pys, nys, pzs, nzs;
		std::memcpy(&pxs, container.payload.data(), 8);
		std::memcpy(&nxs, container.payload.data() + 8, 8);
		std::memcpy(&pys, container.payload.data() + 16, 8);
		std::memcpy(&nys, container.payload.data() + 24, 8);
		std::memcpy(&pzs, container.payload.data() + 32, 8);
		std::memcpy(&nzs, container.payload.data() + 40, 8);

		//Extract encoded face buffers
		std::vector<char> pxFB(pxs), nxFB(nxs), pyFB(pys), nyFB(nys), pzFB(pzs), nzFB(nzs);
		std::size_t offsetCounter = 48;
		CheckException(container.payload.size() >= (48 + pxs + nxs + pys + nys + pzs + nzs), "Cubemap packed container is too small to contain face data of given sizes!");
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
		std::array<libcacaoimage::Image, 6> out {};
		ibytestream pxStream(pxFB);
		out[0] = libcacaoimage::decode::DecodeGeneric(pxStream);
		ibytestream nxStream(nxFB);
		out[1] = libcacaoimage::decode::DecodeGeneric(nxStream);
		ibytestream pyStream(pyFB);
		out[2] = libcacaoimage::decode::DecodeGeneric(pyStream);
		ibytestream nyStream(nyFB);
		out[3] = libcacaoimage::decode::DecodeGeneric(nyStream);
		ibytestream pzStream(pzFB);
		out[4] = libcacaoimage::decode::DecodeGeneric(pzStream);
		ibytestream nzStream(nzFB);
		out[5] = libcacaoimage::decode::DecodeGeneric(nzStream);

		//Return result
		return out;
	}

	std::vector<unsigned char> PackedDecoder::DecodeShader(const PackedContainer& container) {
		CheckException(container.format == PackedFormat::Shader, "Packed container provided for shader decoding is not a shader!");
		CheckException(container.payload.size() > 5, "Shader packed container is too small to contain code data!");

		//Get blob size
		uint32_t blobSize = 0;
		std::memcpy(&blobSize, container.payload.data(), 4);
		CheckException(blobSize > 0, "Shader packed container has no code!");
		CheckException(container.payload.size() > (4 + blobSize), "Shader is not large enough to contain code blob of specified size!");

		//Create result
		std::vector<unsigned char> blob(blobSize);

		//Read data
		std::memcpy(blob.data(), container.payload.data() + 4, blobSize);

		//Return result
		return blob;
	}

	Material PackedDecoder::DecodeMaterial(const PackedContainer& container) {
		CheckException(container.format == PackedFormat::Material, "Packed container provided for material decoding is not a material!");
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
		for(uint8_t i = 0; i < numKeys; ++i) {
			//Get key name
			CheckException(container.payload.size() > offsetCounter + 1, "Material packed container is too small to contain key name length!");
			uint8_t keyNameLen = 0;
			std::memcpy(&keyNameLen, container.payload.data() + offsetCounter++, 1);
			CheckException(container.payload.size() > offsetCounter + 1, "Material packed container is too small to contain key name string of provided length!");
			std::string keyName("\0", keyNameLen);
			std::memcpy(keyName.data(), container.payload.data() + offsetCounter, keyNameLen);
			offsetCounter += keyNameLen;

			//Get type info
			CheckException(container.payload.size() > offsetCounter + 1, "Material packed container is too small to contain provided key count!");
			uint8_t typeInfo = 0;
			std::memcpy(&typeInfo, container.payload.data() + offsetCounter++, 1);

			//Extract info and check size constraints
			Vec2<uint8_t> size;
			size.x = ((typeInfo & 0b00001100) >> 2) + 1;
			size.y = ((typeInfo & 0b00110000) >> 4) + 1;
			uint8_t baseType = (typeInfo & 0b00000011);
			CheckException(baseType != 3 || (baseType == 3 && size.x == 1 && size.y == 1), "Material packed container key has invalid size for texture type (must be 1x1)!");
			CheckException(baseType == 2 || (baseType != 2 && size.x == 1), "Material packed container key has invalid size for non-float type (y must be 1)!");
			CheckException(size.y != 1 || (size.x == 1 && size.y == 1), "Material packed container key has invalid size (if x is 1, must be 1x1)");
			uint8_t dims = (4 * size.x) - (4 - size.y);

			//Load data
			if(baseType != 3) CheckException(container.payload.size() > offsetCounter + (size.x * size.y * 4), "Material packed container key is too small to contain value of provided type!");
			switch(baseType) {
				case 0:
					switch(dims) {
						case 1: {
							int32_t val = 0;
							std::memcpy(&val, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							out.keys.insert_or_assign(keyName, val);
							break;
						}
						case 2: {
							int32_t x = 0, y = 0;
							std::memcpy(&x, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&y, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							out.keys.insert_or_assign(keyName, Vec2<int> {.x = x, .y = y});
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
							out.keys.insert_or_assign(keyName, Vec3<int> {.x = x, .y = y, .z = z});
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
							out.keys.insert_or_assign(keyName, Vec4<int> {.x = x, .y = y, .z = z, .w = w});
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
							out.keys.insert_or_assign(keyName, val);
							break;
						}
						case 2: {
							uint32_t x = 0, y = 0;
							std::memcpy(&x, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&y, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							out.keys.insert_or_assign(keyName, Vec2<unsigned int> {.x = x, .y = y});
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
							out.keys.insert_or_assign(keyName, Vec3<unsigned int> {.x = x, .y = y, .z = z});
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
							out.keys.insert_or_assign(keyName, Vec4<unsigned int> {.x = x, .y = y, .z = z, .w = w});
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
							out.keys.insert_or_assign(keyName, val);
							break;
						}
						case 2: {
							float x = 0, y = 0;
							std::memcpy(&x, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							std::memcpy(&y, container.payload.data() + offsetCounter, 4);
							offsetCounter += 4;
							out.keys.insert_or_assign(keyName, Vec2<float> {.x = x, .y = y});
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
							out.keys.insert_or_assign(keyName, Vec3<float> {.x = x, .y = y, .z = z});
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
							out.keys.insert_or_assign(keyName, Vec4<float> {.x = x, .y = y, .z = z, .w = w});
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
							out.keys.insert_or_assign(keyName, m);
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
							out.keys.insert_or_assign(keyName, m);
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
							out.keys.insert_or_assign(keyName, m);
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
							out.keys.insert_or_assign(keyName, m);
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
							out.keys.insert_or_assign(keyName, m);
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
							out.keys.insert_or_assign(keyName, m);
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
							out.keys.insert_or_assign(keyName, m);
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
							out.keys.insert_or_assign(keyName, m);
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
							out.keys.insert_or_assign(keyName, m);
							break;
						}
						default:
							break;
					}
					break;
				case 3: {
					Material::TextureRef ref {};
					CheckException(container.payload.size() > offsetCounter + 1, "Material packed container key is too small to contain texture reference string length!");
					uint8_t texLen = 0;
					std::memcpy(&texLen, container.payload.data() + offsetCounter++, 1);
					CheckException(container.payload.size() > offsetCounter + 1, "Material packed container key is too small to contain texture reference string of provided length!");
					ref.path = std::string("\0", texLen);
					std::memcpy(ref.path.data(), container.payload.data() + offsetCounter, texLen);
					offsetCounter += texLen;
					ref.isCubemap = (typeInfo & 0b01000000) > 0;
					out.keys.insert_or_assign(keyName, ref);
					break;
				}
			}

			//Advance by two extra bytes since we put two null bytes as a separator between keys
			offsetCounter += 2;
		}

		return out;
	}

	World PackedDecoder::DecodeWorld(const PackedContainer& container) {
		CheckException(container.format == PackedFormat::World, "Packed container provided for world decoding is not a world!");
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

		//Get actor count
		CheckException(container.payload.size() > advance + 8, "World packed container is too small to contain actor data!");
		uint64_t actorCount = 0;
		std::memcpy(&actorCount, container.payload.data() + advance, 8);
		advance += 8;

		//If we don't need to process actors, return value now
		if(actorCount == 0) return out;

		for(uint64_t i = 0; i < actorCount; ++i) {
			World::Actor ent {};

			//Read GUID bytes
			CheckException(container.payload.size() > advance + 16, "World packed container actor data is too small to contain GUID!");
			std::array<uint8_t, 16> guidBytes;
			std::memcpy(guidBytes.data(), container.payload.data() + advance, 16);
			advance += 16;
			ent.guid = xg::Guid(guidBytes);

			//Read parent GUID bytes
			CheckException(container.payload.size() > advance + 16, "World packed container actor data is too small to contain parent GUID!");
			std::array<uint8_t, 16> parentGuidBytes;
			std::memcpy(parentGuidBytes.data(), container.payload.data() + advance, 16);
			advance += 16;
			ent.parentGUID = xg::Guid(parentGuidBytes);

			//Get actor name
			CheckException(container.payload.size() > advance + 2, "World packed container actor data is too small to contain name string length data!");
			uint16_t nameLen = 0;
			std::memcpy(&nameLen, container.payload.data() + advance, 2);
			advance += 2;
			CheckException(nameLen > 0, "World packed container actor data has zero-length name string");
			CheckException(container.payload.size() > advance + nameLen, "World packed container actor data is too small to contain name string!");
			ent.name = std::string("\0", nameLen);
			std::memcpy(ent.name.data(), container.payload.data() + advance, nameLen);
			advance += nameLen;

			//Get initial position, rotation, and scale vectors
			CheckException(container.payload.size() > advance + 36, "World packed container actor data is too small to contain initial transform!");
			std::memcpy(&ent.initialPos.x, container.payload.data() + advance, 4);
			advance += 4;
			std::memcpy(&ent.initialPos.y, container.payload.data() + advance, 4);
			advance += 4;
			std::memcpy(&ent.initialPos.z, container.payload.data() + advance, 4);
			advance += 4;
			std::memcpy(&ent.initialRot.x, container.payload.data() + advance, 4);
			advance += 4;
			std::memcpy(&ent.initialRot.y, container.payload.data() + advance, 4);
			advance += 4;
			std::memcpy(&ent.initialRot.z, container.payload.data() + advance, 4);
			advance += 4;
			std::memcpy(&ent.initialScale.x, container.payload.data() + advance, 4);
			advance += 4;
			std::memcpy(&ent.initialScale.y, container.payload.data() + advance, 4);
			advance += 4;
			std::memcpy(&ent.initialScale.z, container.payload.data() + advance, 4);
			advance += 4;

			//Get component count
			CheckException(container.payload.size() > advance, "World packed container actor data is too small to contain component data!");
			uint8_t componentCount = 0;
			std::memcpy(&componentCount, container.payload.data() + advance++, 1);

			//If we don't need to process components, skip that part
			if(componentCount == 0) goto add_actor;

			for(uint8_t j = 0; j < componentCount; j++) {
				World::Component comp {};

				//Get component type ID
				CheckException(container.payload.size() > advance + 2, "World packed container component data is too small to contain type ID string length data!");
				uint16_t typeIdLen = 0;
				std::memcpy(&typeIdLen, container.payload.data() + advance, 2);
				advance += 2;
				CheckException(typeIdLen > 0, "World packed container component data has zero-length type ID string");
				CheckException(container.payload.size() > advance + typeIdLen, "World packed container component data is too small to contain type ID string!");
				comp.typeID = std::string("\0", typeIdLen);
				std::memcpy(comp.typeID.data(), container.payload.data() + advance, typeIdLen);
				advance += typeIdLen;

				//Get reflection data
				CheckException(container.payload.size() > advance + 4, "World packed container component data is too small to contain reflection size data!");
				uint32_t rflLen = 0;
				std::memcpy(&rflLen, container.payload.data() + advance, 4);
				advance += 4;
				CheckException(rflLen > 0, "World packed container component data has zero-length reflection data");
				CheckException(container.payload.size() > advance + rflLen, "World packed container component data is too small to contain reflection data of provided size!");
				comp.reflection = std::string("\0", rflLen);
				std::memcpy(comp.reflection.data(), container.payload.data() + advance, rflLen);
				advance += rflLen;

				//We put 2 null bytes as a separator between components for padding purposes, so skip over that
				advance += 2;

				//Add component to actor
				ent.components.push_back(comp);
			}

		add_actor:
			//We put "%e" as a separator between actors for padding purposes, so skip over that
			advance += 2;

			//Add actor to world
			out.actors.push_back(ent);
		}

		return out;
	}

	AssetPack PackedDecoder::DecodeAssetPack(const PackedContainer& container) {
		CheckException(container.format == PackedFormat::AssetPack, "Packed container provided for asset pack decoding is not an asset pack!");

		//Configure archive object
		archive* pak = archive_read_new();
		CheckException(pak, "Unable to create asset pack archive object!");
		archive_read_support_format_tar(pak);
		CheckException(archive_read_open_memory(pak, container.payload.data(), container.payload.size() * sizeof(uint8_t)) == ARCHIVE_OK, "Failed to open asset pack archive data!");

		//Create output
		AssetPack out;

		//Create check map
		std::map<std::string, bool> check;

		//Extract data
		archive_entry* entry;
		YAML::Node metaRoot(YAML::NodeType::Undefined);
		while(archive_read_next_header(pak, &entry) == ARCHIVE_OK) {
			std::filesystem::path path(archive_entry_pathname_utf8(entry));
			std::string filename = path.filename().string();

			//Check if this is the reserved metadata file (the name is weird to reduce the possibility of conflicts)
			if(filename.compare("__$CacaoMeta0") == 0) {
				//Get file size
				la_int64_t size = archive_entry_size(entry);

				//Read data from entry into buffer
				std::vector<char> metaBuf(size);
				archive_read_data(pak, metaBuf.data(), size);

				//Load metadata (using byte stream to do easy feed-in from vector)
				ibytestream input(metaBuf);
				metaRoot = YAML::Load(input);
			} else {
				//Get file size
				la_int64_t size = archive_entry_size(entry);

				//Create PackedAsset. We set everything to a Resource by default because we need something. This gets corrected in the second pass with info from the metadata file if applicable.
				PackedAsset asset {.kind = PackedAsset::Kind::Resource, .buffer = std::vector<unsigned char>(size)};

				//If this is a resource, we need to remove the resource path prefix
				std::string genericPathStr = path.generic_string();
				bool shouldStayRes = false;
				if(genericPathStr.starts_with("__$CacaoRes/")) {
					shouldStayRes = true;
					path = std::filesystem::path(genericPathStr.substr(12, genericPathStr.size()));
				}

				//Read data from entry into buffer
				archive_read_data(pak, asset.buffer.data(), size);

				//Create reference name
				std::string reference = shouldStayRes ? path.string() : filename;

				//Add entry to output
				out.insert_or_assign(reference, asset);
				check.insert_or_assign(reference, shouldStayRes);
			}
		}

		//Check that we actually got a metadata file
		ValidateYAMLNode(metaRoot, YAML::NodeType::value::Sequence, "asset pack", "Metadata file does not exist in pack or is improperly formatted!");

		//Get asset types from metadata file
		for(const YAML::Node& meta : metaRoot) {
			//Validate metadata entry structure
			ValidateYAMLNode(meta, YAML::NodeType::value::Map, [](const YAML::Node& node) {
				if(!node["asset"].IsDefined() || !node["asset"].IsScalar()) return "asset parameter is of invalid type or does not exist";
				if(!node["type"].IsDefined() || !node["type"].IsScalar()) return "type parameter is of invalid type or does not exist";
				try {
					if(int asInt = std::stoi(node["type"].Scalar()); asInt < 0 || asInt > 6) return "type parameter is out of range";
				} catch(...) {
					return "type parameter is not an integer";
				}
				return ""; }, "asset pack metadata file node", "Metadata entry is not a map!");

			//Extract info
			std::string asset = meta["asset"].Scalar();
			int type = std::stoi(meta["type"].Scalar());

			CheckException(out.contains(asset), "Metadata file references non-existent asset!");

			//Set asset type
			switch(type) {
				case 0:
					out.at(asset).kind = PackedAsset::Kind::Shader;
					break;
				case 1:
					out.at(asset).kind = PackedAsset::Kind::Tex2D;
					break;
				case 2:
					out.at(asset).kind = PackedAsset::Kind::Cubemap;
					break;
				case 3:
					out.at(asset).kind = PackedAsset::Kind::Sound;
					break;
				case 4:
					out.at(asset).kind = PackedAsset::Kind::Material;
					break;
				case 5:
					out.at(asset).kind = PackedAsset::Kind::Font;
					break;
				case 6:
					out.at(asset).kind = PackedAsset::Kind::Model;
					break;
			}

			//Mark as checked
			check.insert_or_assign(asset, true);
		}

		//Confirm all assets checked
		for(const auto& pair : check) {
			CheckException(pair.second, "Asset pack contains file with no metadata!");
		}

		return out;
	}
}