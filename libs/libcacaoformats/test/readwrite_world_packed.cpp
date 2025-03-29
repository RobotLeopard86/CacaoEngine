#include "libcacaoformats/libcacaoformats.hpp"

#include <fstream>
#include <iostream>

int main() {
	try {
		//Write it
		{
			libcacaoformats::World w;
			w.skyboxRef = "aSkybox";
			w.initialCamPos = libcacaoformats::Vec3<float> {.x = 7, .y = 5.9, .z = -1.1234};
			w.initialCamRot = libcacaoformats::Vec3<float> {.x = 0, .y = -45, .z = 0};
			libcacaoformats::World::Entity& e = w.entities.emplace_back();
			e.name = "Honk";
			e.parentGUID = "00000000-0000-0000-0000-000000000000";
			e.guid = "36f595e7-e072-4219-bd0d-7db2d9260eea";
			e.initialPos = libcacaoformats::Vec3<float> {.x = 1, .y = 0, .z = -1.4};
			e.initialRot = libcacaoformats::Vec3<float> {.x = 0, .y = 0, .z = 0};
			e.initialScale = libcacaoformats::Vec3<float> {.x = 2, .y = 7, .z = 3};
			libcacaoformats::World::Component& c1 = e.components.emplace_back();
			c1.typeID = "aType";
			c1.reflection = "someProp: 1.3";
			libcacaoformats::World::Component& c2 = e.components.emplace_back();
			c2.typeID = "aDifferentType";
			c2.reflection = "theVec:\n  x: 2.1\n  y: -8.47";
			libcacaoformats::PackedEncoder enc;
			std::ofstream str("./out.xcw", std::ios::binary);
			enc.EncodeWorld(w).ExportToStream(str);
			str.close();
		}

		//Read it
		{
			std::ifstream str("./out.xcw", std::ios::binary);
			libcacaoformats::PackedDecoder dec;
			libcacaoformats::PackedContainer container = libcacaoformats::PackedContainer::FromStream(str);
			libcacaoformats::World w = dec.DecodeWorld(container);
			str.close();
			if(w.skyboxRef.compare("aSkybox") != 0) throw std::runtime_error("Wrong skybox reference!");
			if(w.initialCamPos.x != 7 || w.initialCamPos.y != 5.9 || w.initialCamPos.z != -1.1234) throw std::runtime_error("Wrong initial camera position!");
			if(w.initialCamRot.x != 0 || w.initialCamRot.y != -45 || w.initialCamRot.z != 0) throw std::runtime_error("Wrong initial camera rotation!");
			if(w.entities.size() != 1) throw std::runtime_error("Wrong amount of entities!");
			libcacaoformats::World::Entity& e = w.entities[0];
			if(e.name.compare("Honk") != 0) throw std::runtime_error("Wrong entity name!");
			if(e.guid.compare("36f595e7-e072-4219-bd0d-7db2d9260eea") != 0) throw std::runtime_error("Wrong entity GUID!");
			if(e.parentGUID.compare("00000000-0000-0000-0000-000000000000") != 0) throw std::runtime_error("Wrong entity parent GUID!");
			if(e.initialPos.x != 1 || e.initialPos.y != 0 || e.initialPos.z != -1.4) throw std::runtime_error("Wrong initial entity position!");
			if(e.initialRot.x != 0 || e.initialRot.y != 0 || e.initialRot.z != 0) throw std::runtime_error("Wrong initial entity rotation!");
			if(e.initialScale.x != 2 || e.initialScale.y != 7 || e.initialScale.z != 3) throw std::runtime_error("Wrong initial entity scale!");
			if(e.components.size() != 2) throw std::runtime_error("Wrong amount of components!");
			libcacaoformats::World::Component& c1 = e.components[0];
			libcacaoformats::World::Component& c2 = e.components[1];
			if(c1.typeID.compare("aType") != 0) throw std::runtime_error("Wrong first component type ID!");
			if(c1.reflection.compare("someProp: 1.3") != 0) throw std::runtime_error("Wrong first component reflection data!");
			if(c2.typeID.compare("aDifferentType") != 0) throw std::runtime_error("Wrong second component type ID!");
			if(c2.reflection.compare("theVec:\n  x: 2.1\n  y: -8.47") != 0) throw std::runtime_error("Wrong second component reflection data!");
		}

		return 0;
	} catch(const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
}