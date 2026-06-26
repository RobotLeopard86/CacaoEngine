#include "libcacaoasset.hpp"

namespace libcacaoasset {
	bool BaseResAddrCheck(std::string check, std::string specialAllow = "") {
		//Restrict character set
		std::string allowed = specialAllow + "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_:";
		if(check.find_first_not_of(allowed) != std::string::npos) return false;

		//Ensure type prefix is alphabetical (ASCII magic)
		if(check[0] == '_' || check[0] <= 58) return false;

		//Ensure type and identifier separator exists
		if(check[1] != ':') return false;

		//Check for excess separators (first part removes colon)
		allowed.pop_back();
		allowed.shrink_to_fit();
		if(check.substr(2).find_first_not_of(allowed) != std::string::npos) return false;

		return true;
	}

	bool ValidateResourceAddress_Tex2D(const std::string& addr) {
		return BaseResAddrCheck(addr, addr[0] == 'e' ? "/" : "") && (addr[0] == 'a' || (addr[0] == 'e' && addr.find_first_of('/') == addr.find_last_of('/')));
	}

	bool ValidateResourceAddress_Mesh(const std::string& addr) {
		return BaseResAddrCheck(addr, "/") && addr[0] == 'm' && addr.find_first_of('/') == addr.find_last_of('/');
	}

	bool ValidateResourceAddress_AssetGeneric(const std::string& addr) {
		return BaseResAddrCheck(addr) && addr[0] == 'a';
	}

	bool ValidateResourceAddress_World(const std::string& addr) {
		return BaseResAddrCheck(addr) && addr[0] == 'w';
	}

	bool ValidateResourceAddress_Blob(const std::string& addr) {
		return BaseResAddrCheck(addr, "./") && addr[0] == 'r' && addr.find("..") == std::string::npos && addr.find("//") == std::string::npos && addr.find("./") == std::string::npos;
	}

	bool ValidateResourceAddress(const std::string& address, Resource::Type type) {
		switch(type) {
			case Resource::Type::Blob:
				return ValidateResourceAddress_Blob(address);
			case Resource::Type::Mesh:
				return ValidateResourceAddress_Mesh(address);
			case Resource::Type::Tex2D:
				return ValidateResourceAddress_Tex2D(address);
			case Resource::Type::World:
				return ValidateResourceAddress_World(address);
			case Resource::Type::Shader:
			case Resource::Type::Material:
			case Resource::Type::Cubemap:
			case Resource::Type::Audio:
			case Resource::Type::Font:
			case Resource::Type::Model:
				return ValidateResourceAddress_AssetGeneric(address);
		}
	}
}