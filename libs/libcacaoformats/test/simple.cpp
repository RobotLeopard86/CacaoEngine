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
			std::cout << mat.shader << std::endl;
			if(mat.keys.size() != 2) throw std::runtime_error("Wrong amount of keys!");
			for(const auto& k : mat.keys) {
				std::cout << k.first << ":\n"
						  << k.second.index() << "\n";
				if(k.second.index() != 4 && k.second.index() != 13) throw std::runtime_error("Loaded key is not the correct index");
				if(k.second.index() == 4) {
					libcacaoformats::Vec3<int> val = std::get<4>(k.second);
					std::cout << "{" << val.x << ", " << val.y << ", " << val.z << "}" << std::endl;
				} else {
					libcacaoformats::Matrix<float, 2, 3> val = std::get<13>(k.second);
					for(const auto& row : val.data) {
						std::cout << "[" << row[0] << ", " << row[1] << "]" << std::endl;
					}
				}
			}
		}

		return 0;
	} catch(const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
}