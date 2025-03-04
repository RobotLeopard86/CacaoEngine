#include "libcacaoformats/libcacaoformats.hpp"

#include <fstream>
#include <iostream>

int main() {
	try {
		//Write it
		{
			libcacaoformats::Material mat;
			mat.shader = "yappucino";
			mat.keys.insert_or_assign("test", libcacaoformats::Vec3<int> {.x = 7, .y = -4, .z = 3});
			libcacaoformats::Matrix<float, 2, 3> trix;
			trix[0] = {2.4f, 3.1f};
			trix[1] = {66.1f, 9.143f};
			trix[2] = {100.0f, 31.4f};
			mat.keys.insert_or_assign("goose", trix);
			libcacaoformats::PackedEncoder enc;
			std::ofstream str("./out.cacao", std::ios::binary);
			enc.EncodeMaterial(mat).ExportToStream(str);
			str.close();
		}

		//Read it
		{
			std::ifstream str("./out.cacao", std::ios::binary);
			libcacaoformats::PackedDecoder dec;
			libcacaoformats::PackedContainer container = libcacaoformats::PackedContainer::FromStream(str);
			libcacaoformats::Material mat = dec.DecodeMaterial(container);
			str.close();
			if(mat.shader.compare("yappucino") != 0) throw std::runtime_error("Wrong shader reference!");
			if(mat.keys.size() != 2) throw std::runtime_error("Wrong amount of keys!");
			if(!mat.keys.contains("goose") || !mat.keys.contains("test")) throw std::runtime_error("Wrong keys decoded!");
			libcacaoformats::Vec3<int> test = std::get<4>(mat.keys.at("test"));
			libcacaoformats::Matrix<float, 2, 3> goose = std::get<13>(mat.keys.at("test"));
			if(test.x != 7 || test.y != -4 || test.z != 3) throw std::runtime_error("\"test\" key contains wrong values!");
			if(goose[0][0] != 2.4f || goose[0][1] != 3.1f || goose[1][0] != 66.1f || goose[1][1] != 9.143f || goose[2][0] != 100.0f || goose[2][1] != 31.4f) throw std::runtime_error("\"goose\" key contains wrong values!");
		}

		return 0;
	} catch(const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
}