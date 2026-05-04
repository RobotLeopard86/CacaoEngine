#include "libcacaoasset.hpp"
#include "libcacaocommon.hpp"

#include "libjaguar/Document.hpp"
#include "libjaguar/Index.hpp"
#include "libjaguar/StructuredTypeLayout.hpp"
#include "libjaguar/TypeTags.hpp"

#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>

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
		CheckException(stream, "Invalid stream pointer!");
		CheckException(stream->good(), "Stream is broken!");

		//Wrap in unique_ptr
		std::unique_ptr<std::istream> ptr(stream);

		//Create object and document
		AssetPack pak;
		pak.doc = libjaguar::Document(std::move(ptr));

		//Register resource type (will also trigger implicit document parsing)
		libjaguar::StructuredTypeLayout resourceLayout = {};
		{
			libjaguar::StructuredTypeLayout::Field& id = resourceLayout.fields.emplace_back();
			id.name = "id";
			id.type = libjaguar::TypeTag::String;
		}
		{
			libjaguar::StructuredTypeLayout::Field& type = resourceLayout.fields.emplace_back();
			type.name = "type";
			type.type = libjaguar::TypeTag::UInt8;
		}
		{
			libjaguar::StructuredTypeLayout::Field& bytes = resourceLayout.fields.emplace_back();
			bytes.name = "bytes";
			bytes.type = libjaguar::TypeTag::ByteBuffer;
		}
		pak.doc.RegisterStructuredObjConverter<Resource>("Resource", resourceLayout, _DecResource, _EncResource);

		//Verify root structure
		CheckException(pak.doc.HasValue("root"), "Asset pack is malformed; does not have root section!");
		libjaguar::ScopeEntry root = pak.doc.QueryScopeInfo("root");
		CheckException(root.list, "Asset pack is malformed; root section is not of correct type!");
		CheckException(root.typeID.compare("Resource") == 0, "Asset pack is malformed; root section is not of correct type!");

		return pak;
	}

	Resource AssetPack::GetResource(const std::string& address) {
		CheckException(address.starts_with("a:") || address.starts_with("r:"), "Invalid resource address!");
		std::string realpath = std::format("root.{}", address.substr(2));
		CheckException(doc.HasValue(realpath), "Asset pack does not contain requested resource!");

		return doc.QueryValue<Resource>(realpath);
	}

	void AssetPack::Export(std::ostream* stream) {
		CheckException(stream, "Invalid stream pointer!");
		CheckException(stream->good(), "Stream is broken!");

		//Export via Document interface
		doc.ExportTo(*stream);
	}

	Resource AssetPack::_DecResource(libjaguar::Document::ObjReader& rd) {
		Resource out;
		out.id = rd.Get<std::string>("id");
		out.bytes = rd.Get<std::vector<unsigned char>>("bytes");
		uint8_t typeNum = rd.Get<uint8_t>("type");
		if(typeNum < 0 || typeNum > 6) throw std::runtime_error("Invalid type number!");
		out.type = static_cast<Resource::Type>(typeNum);
		return out;
	}

	void AssetPack::_EncResource(const Resource& r, libjaguar::Document::ObjWriter& wr) {
		CheckException(!r.id.empty(), "Invalid resource ID!");
		CheckException(!r.bytes.empty(), "Resources cannot be empty!");

		wr.Set<std::string>("id", r.id);
		wr.Set<std::vector<unsigned char>>("bytes", r.bytes);
		wr.Set<unsigned char>("type", static_cast<uint8_t>(r.type));
	}
}