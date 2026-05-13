#include "libcacaoasset.hpp"
#include "libcacaocommon.hpp"

#include "libjaguar/Document.hpp"
#include "libjaguar/Index.hpp"
#include "libjaguar/StructuredTypeLayout.hpp"
#include "libjaguar/TypeTags.hpp"

#include <filesystem>
#include <fstream>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <cstdint>

#define XAK_REVISION uint16_t(1)

namespace libcacaoasset {
	AssetPack AssetPack::OpenFromFile(const std::string& filePath) {
		CheckException(std::filesystem::exists(filePath), "File does not exist!");

		//Open stream
		std::ifstream* filestream = new std::ifstream(filePath);
		CheckException(filestream && filestream->is_open(), "Failed to open asset pack file!");

		//We're all good, send to common stream logic
		return OpenFromStream(filestream);
	}

	void AssetPack::RegisterResourceType() {
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
		doc.RegisterStructuredObjConverter<Resource>("Resource", resourceLayout, _DecResource, _EncResource);
	}

	AssetPack AssetPack::OpenFromStream(std::istream* stream) {
		CheckException(stream, "Invalid stream pointer!");
		CheckException(stream->good(), "Stream is broken!");

		//Check for header
		std::array<char, 6> headerChk;
		stream->read(headerChk.data(), headerChk.size());
		CheckException(stream->good(), "Failed to read asset pack header!");
		CheckException(headerChk[0] == 'x' && headerChk[1] == 'a' && headerChk[2] == 'k' && headerChk[3] == 'f' && headerChk[4] == 'i' && headerChk[5] == 'l', "Invalid asset pack header");

		//Check file revision
		uint16_t revision = 0;
		revision |= stream->get();
		CheckException(stream->good(), "Failed to read asset pack version stamp!");
		revision |= (stream->get() << 8);
		CheckException(stream->good(), "Failed to read asset pack version stamp!");
		CheckException(revision <= XAK_REVISION, "Asset pack is of incompatible revision!");

		//Wrap in unique_ptr
		std::unique_ptr<std::istream> ptr(stream);

		//Create object and document
		AssetPack pak;
		pak.doc = libjaguar::Document(std::move(ptr));

		//Register resource type (will also trigger implicit document parsing)
		pak.RegisterResourceType();

		//Verify root structure
		libjaguar::ScopeEntry docRoot = pak.doc.QueryScopeInfo("");
		CheckException(docRoot.subscopes.size() == 2 && docRoot.subvalues.size() == 0, "Asset pack is malformed; missing required fields or has extra fields!");
		CheckException(pak.doc.HasValue("aRoot"), "Asset pack is malformed; does not have root assets section!");
		CheckException(pak.doc.HasValue("rRoot"), "Asset pack is malformed; does not have root resources section!");
		libjaguar::ScopeEntry aRoot = pak.doc.QueryScopeInfo("aRoot");
		CheckException(aRoot.list, "Asset pack is malformed; root assets section is not of correct type!");
		CheckException(aRoot.typeID.compare("Resource") == 0, "Asset pack is malformed; root assets section is not of correct type!");
		libjaguar::ScopeEntry rRoot = pak.doc.QueryScopeInfo("rRoot");
		CheckException(rRoot.list, "Asset pack is malformed; root resources section is not of correct type!");
		CheckException(rRoot.typeID.compare("Resource") == 0, "Asset pack is malformed; root resources section is not of correct type!");

		return pak;
	}

	AssetPack AssetPack::CreateEmpty() {
		//Create base object and document
		AssetPack pak;
		pak.doc = libjaguar::Document();

		//Register resource type
		pak.RegisterResourceType();

		//Create empty root structures
		pak.doc.CreateValue<std::vector<Resource>>("aRoot");
		pak.doc.CreateValue<std::vector<Resource>>("rRoot");

		return pak;
	}

	Resource AssetPack::GetResource(const std::string& address) {
		CheckException(address.starts_with("a:") || address.starts_with("r:"), "Invalid resource address!");

		//Find the resource
		std::string tgt = std::format("{}Root", address[0]);
		auto filtered = std::views::filter(doc.QueryScopeInfo(tgt).subscopes, [this, tgt, address](const libjaguar::ScopeEntry& entry) -> bool {
			return doc.QueryValue<std::string>(std::format("{}[{}].id", tgt, entry.name)).compare(address.substr(2)) == 0;
		}) | std::views::common;
		std::vector<libjaguar::ScopeEntry> result(filtered.begin(), filtered.end());
		CheckException(result.size() == 1, "Could not locate asset!");

		//Fetch real output
		Resource ret = doc.QueryValue<Resource>(std::format("{}[{}]", tgt, result[0].name));

		//Validate address is actually correct
		//It feels weird to do this after fetching but we need to know the type to validate so
		CheckException(ValidateResourceAddress(address, ret.type), "Invalid resource address!");

		return ret;
	}

	std::vector<std::string> AssetPack::ListResources() {
		libjaguar::ScopeEntry aRoot = doc.QueryScopeInfo("aRoot");
		libjaguar::ScopeEntry rRoot = doc.QueryScopeInfo("rRoot");
		auto aTransformed = std::views::transform(aRoot.subscopes, [this](const libjaguar::ScopeEntry& entry) -> std::string {
			return "a:" + doc.QueryValue<std::string>(std::format("aRoot[{}].id", entry.name));
		});
		std::vector<std::string> aVec(aTransformed.begin(), aTransformed.end());
		auto rTransformed = std::views::transform(rRoot.subscopes, [this](const libjaguar::ScopeEntry& entry) -> std::string {
			return "r:" + doc.QueryValue<std::string>(std::format("rRoot[{}].id", entry.name));
		});
		std::vector<std::string> rVec(rTransformed.begin(), rTransformed.end());
		auto transformed = std::views::join(std::vector<std::vector<std::string>> {aVec, rVec}) | std::views::common;
		return std::vector(transformed.begin(), transformed.end());
	}

	std::vector<std::string> AssetPack::ListResourcesOfType(Resource::Type type) {
		CheckException(type != Resource::Type::Mesh && type != Resource::Type::World, "Invalid resource type!");
		if(type == Resource::Type::Blob) {
			auto transformed = std::views::transform(doc.QueryScopeInfo("rRoot").subscopes, [this](const libjaguar::ScopeEntry& entry) -> std::string {
				return "r:" + doc.QueryValue<std::string>(std::format("rRoot[{}].id", entry.name));
			}) | std::views::common;
			return std::vector<std::string>(transformed.begin(), transformed.end());
		} else {
			auto transformed = std::views::filter(doc.QueryScopeInfo("aRoot").subscopes, [this, type](const libjaguar::ScopeEntry& entry) -> bool {
				return doc.QueryValue<uint8_t>(std::format("aRoot[{}].type", entry.name)) == static_cast<uint8_t>(type);
			}) | std::views::transform([this](const libjaguar::ScopeEntry& entry) -> std::string {
				return "a:" + doc.QueryValue<std::string>(std::format("aRoot[{}].id", entry.name));
			}) | std::views::common;
			return std::vector<std::string>(transformed.begin(), transformed.end());
		}
	}

	void AssetPack::PutResource(const std::string& address, Resource&& resource) {
		CheckException(ValidateResourceAddress(address, resource.type), "Invalid resource address!");
		CheckException(resource.type != Resource::Type::World && resource.type != Resource::Type::Mesh, "Invalid resource type!");
		if(resource.type == Resource::Type::Tex2D) CheckException(address[0] != 'm', "Cannot use model address format for direct texture resource address!");

		//Find the resource
		std::string tgt = std::format("{}Root", address[0]);
		auto filtered = std::views::filter(doc.QueryScopeInfo(tgt).subscopes, [this, tgt, address](const libjaguar::ScopeEntry& entry) -> bool {
			return doc.QueryValue<std::string>(std::format("{}[{}].id", tgt, entry.name)).compare(address.substr(2)) == 0;
		}) | std::views::common;
		std::vector<libjaguar::ScopeEntry> result(filtered.begin(), filtered.end());
		CheckException(result.size() <= 1, "Broken asset pack: duplicate entries!");

		//Set value in the document
		if(result.size() == 0) {
			doc.SetOrCreateValue<Resource>(std::format("{}[{}]", tgt, doc.QueryScopeInfo(tgt).subscopes.size()), resource);
		} else {
			doc.SetOrCreateValue<Resource>(std::format("{}[{}]", tgt, result[0].name), resource);
		}
	}

	void AssetPack::Export(std::ostream* stream) {
		CheckException(stream, "Invalid stream pointer!");
		CheckException(stream->good(), "Stream is broken!");

		//Write header
		stream->write("xakfil", 6);
		stream->put(XAK_REVISION & 0xF);
		stream->put(XAK_REVISION >> 8);

		//Export body via Document interface
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
		wr.Set<uint8_t>("type", static_cast<uint8_t>(r.type));
	}
}