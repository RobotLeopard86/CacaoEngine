#include "libcacaoasset.hpp"
#include "libcacaocommon.hpp"
#include "libjaguar/Document.hpp"

#include <filesystem>
#include <fstream>
#include <memory>

#define MAX_XAK_REVISION 1

namespace libcacaoasset {
	AssetPack AssetPack::OpenFromFile(const std::string& filePath) {
		CheckException(std::filesystem::exists(filePath), "File does not exist!");

		//Open stream
		std::ifstream* filestream = new std::ifstream(filePath);
		CheckException(filestream && filestream->is_open(), "Failed to open asset pack file!");

		//Check for header
		std::array<char, 6> headerChk;
		filestream->read(headerChk.data(), headerChk.size());
		CheckException(filestream->good(), "Failed to read asset pack header!");
		CheckException(headerChk[0] == 'x' && headerChk[1] == 'a' && headerChk[2] == 'k' && headerChk[3] == 'f' && headerChk[4] == 'i' && headerChk[5] == 'l', "Invalid asset pack header");

		//Check file revision
		uint16_t revision = 0;
		revision |= filestream->get();
		CheckException(filestream->good(), "Failed to read asset pack version stamp!");
		revision <<= 8;
		revision |= filestream->get();
		CheckException(filestream->good(), "Failed to read asset pack version stamp!");
		CheckException(revision <= MAX_XAK_REVISION, "Asset pack is of incompatible revision!");

		//We're all good, send to Jaguar
		return OpenFromStream(filestream);
	}

	AssetPack AssetPack::OpenFromStream(std::istream* stream) {
		CheckException(stream->good(), "Stream is broken!");

		//Wrap in unique_ptr
		std::unique_ptr<std::istream> ptr(stream);

		//Create object and document
		AssetPack pak;
		pak.doc = libjaguar::Document(std::move(ptr));

		//TODO: register types (this will trigger parsing)

		//TODO: check strutcture

		return pak;
	}
}