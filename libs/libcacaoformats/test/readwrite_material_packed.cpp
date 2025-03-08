#include "libcacaoformats/libcacaoformats.hpp"

#include <fstream>
#include <iostream>

int main() {
	try {
		//Write it
		{
			libcacaoformats::Material mat;
			mat.shader = "aShader";
			mat.keys.insert_or_assign("test_vec", libcacaoformats::Vec3<int> {.x = 7, .y = -4, .z = 3});
			libcacaoformats::Matrix<float, 2, 3> trix;
			trix[0] = {2.4f, 3.1f};
			trix[1] = {66.1f, 9.143f};
			trix[2] = {100.0f, 31.4f};
			mat.keys.insert_or_assign("test_mat", trix);
			libcacaoformats::PackedEncoder enc;
			std::ofstream str("./out.xcm", std::ios::binary);
			enc.EncodeMaterial(mat).ExportToStream(str);
			str.close();
		}

		//Read it
		{
			std::ifstream str("./out.xcm", std::ios::binary);
			libcacaoformats::PackedDecoder dec;
			libcacaoformats::PackedContainer container = libcacaoformats::PackedContainer::FromStream(str);
			libcacaoformats::Material mat = dec.DecodeMaterial(container);
			str.close();
			if(mat.shader.compare("aShader") != 0) throw std::runtime_error("Wrong shader reference!");
			if(mat.keys.size() != 2) throw std::runtime_error("Wrong amount of keys!");
			if(!mat.keys.contains("test_mat") || !mat.keys.contains("test_vec")) throw std::runtime_error("Wrong keys decoded!");
			libcacaoformats::Vec3<int> testVec = std::get<4>(mat.keys.at("test_vec"));
			libcacaoformats::Matrix<float, 2, 3> testMat = std::get<13>(mat.keys.at("test_mat"));
			if(testVec.x != 7 || testVec.y != -4 || testVec.z != 3) throw std::runtime_error("\"testVec\" key contains wrong values!");
			if(testMat[0][0] != 2.4f || testMat[0][1] != 3.1f || testMat[1][0] != 66.1f || testMat[1][1] != 9.143f || testMat[2][0] != 100.0f || testMat[2][1] != 31.4f) throw std::runtime_error("\"testMat\" key contains wrong values!");
		}

		return 0;
	} catch(const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
}