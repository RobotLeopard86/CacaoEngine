
#include "libcacaoasset.hpp"
#include "CheckException.hpp"

#include "libjaguar/Document.hpp"
#include "libjaguar/Index.hpp"
#include "libjaguar/TypeTags.hpp"
#include <variant>

#define MAT_REVISION uint16_t(1)

namespace libcacaoasset {
	Material::TexRef _DecTexRef(libjaguar::Document::ObjReader& rd) {
		Material::TexRef ref = {};
		ref.address = rd.Query<std::string>("addr");
		ref.isCubemap = rd.Query<bool>("cmap");
		return ref;
	}

	void _EncTexRef(const Material::TexRef& tr, libjaguar::Document::ObjWriter& ow) {
		ow.SetOrCreate<std::string>("addr", tr.address);
		ow.SetOrCreate<bool>("cmap", tr.isCubemap);
	}

	Material::Param _DecParam(libjaguar::Document::ObjReader& rd) {
		Material::Param param = {};
		param.target = rd.Query<std::string>("target");
		const libjaguar::ScopeEntry& storageData = rd.QueryScopeInfo("storage");
		CheckException((storageData.subscopes.size() + storageData.subvalues.size()) == 1, "Bad storage format!");
		CheckException(rd.Has("storage.payload"), "Missing payload information!");
		bool no = false;
		try {
			//If payload is a value, this works, otherwise it throws and we know payload is a scope
			const libjaguar::ValueEntry& payloadData = rd.QueryValueInfo("storage.payload");
			switch(payloadData.type) {
				case libjaguar::TypeTag::SInt32:
					param.storage = rd.Query<int>("storage.payload");
					break;
				case libjaguar::TypeTag::UInt32:
					param.storage = rd.Query<unsigned int>("storage.payload");
					break;
				case libjaguar::TypeTag::Float32:
					param.storage = rd.Query<float>("storage.payload");
					break;
				case libjaguar::TypeTag::Boolean:
					param.storage = rd.Query<bool>("storage.payload");
					break;
				case libjaguar::TypeTag::Matrix:
					if(payloadData.elementType != libjaguar::TypeTag::Float32) {
						no = true;
						break;
					}
					if(payloadData.width != payloadData.height) {
						no = true;
						break;
					}
					switch(payloadData.width) {
						case 2:
							param.storage = rd.Query<libjaguar::Matrix<float, 2, 2>>("storage.payload");
							break;
						case 3:
							param.storage = rd.Query<libjaguar::Matrix<float, 3, 3>>("storage.payload");
							break;
						case 4:
							param.storage = rd.Query<libjaguar::Matrix<float, 4, 4>>("storage.payload");
							break;
					}
					break;
				case libjaguar::TypeTag::Vector:
					switch(payloadData.elementType) {
						case libjaguar::TypeTag::SInt32:
							switch(payloadData.width) {
								case 2:
									param.storage = rd.Query<libjaguar::Vector<int, 2>>("storage.payload");
									break;
								case 3:
									param.storage = rd.Query<libjaguar::Vector<int, 3>>("storage.payload");
									break;
								case 4:
									param.storage = rd.Query<libjaguar::Vector<int, 4>>("storage.payload");
									break;
							}
							break;
						case libjaguar::TypeTag::UInt32:
							switch(payloadData.width) {
								case 2:
									param.storage = rd.Query<libjaguar::Vector<unsigned int, 2>>("storage.payload");
									break;
								case 3:
									param.storage = rd.Query<libjaguar::Vector<unsigned int, 3>>("storage.payload");
									break;
								case 4:
									param.storage = rd.Query<libjaguar::Vector<unsigned int, 4>>("storage.payload");
									break;
							}
							break;
						case libjaguar::TypeTag::Float32:
							switch(payloadData.width) {
								case 2:
									param.storage = rd.Query<libjaguar::Vector<float, 2>>("storage.payload");
									break;
								case 3:
									param.storage = rd.Query<libjaguar::Vector<float, 3>>("storage.payload");
									break;
								case 4:
									param.storage = rd.Query<libjaguar::Vector<float, 4>>("storage.payload");
									break;
							}
							break;
						default:
							no = true;
							break;
					}
					break;
				default:
					no = true;
					break;
			}
		} catch(...) {
			//Guess it's a scope
			//We can just use the TexRef decoder (since that's the only allowable scope); if that fails then this was never going to work
			try {
				param.storage = rd.Query<Material::TexRef>("storage.payload");
			} catch(...) {
				no = true;
			}
		}
		CheckException(!no, "Illegal payload type!");
		return param;
	}

	void _EncParam(const Material::Param& p, libjaguar::Document::ObjWriter& ow) {
		ow.SetOrCreate<std::string>("target", p.target);
		ow.EnsureUnstructuredObjExists("storage");
		switch(p.storage.index()) {
			case 0:
				ow.SetOrCreate<int>("storage.payload", std::get<int>(p.storage));
				break;
			case 1:
				ow.SetOrCreate<unsigned int>("storage.payload", std::get<unsigned int>(p.storage));
				break;
			case 2:
				ow.SetOrCreate<float>("storage.payload", std::get<float>(p.storage));
				break;
			case 3:
				ow.SetOrCreate<bool>("storage.payload", std::get<bool>(p.storage));
				break;
			case 4:
				ow.SetOrCreate<Material::TexRef>("storage.payload", std::get<Material::TexRef>(p.storage));
				break;
			case 5:
				ow.SetOrCreate<libjaguar::Vector<float, 2>>("storage.payload", std::get<libjaguar::Vector<float, 2>>(p.storage));
				break;
			case 6:
				ow.SetOrCreate<libjaguar::Vector<float, 3>>("storage.payload", std::get<libjaguar::Vector<float, 3>>(p.storage));
				break;
			case 7:
				ow.SetOrCreate<libjaguar::Vector<float, 4>>("storage.payload", std::get<libjaguar::Vector<float, 4>>(p.storage));
				break;
			case 8:
				ow.SetOrCreate<libjaguar::Vector<int, 2>>("storage.payload", std::get<libjaguar::Vector<int, 2>>(p.storage));
				break;
			case 9:
				ow.SetOrCreate<libjaguar::Vector<int, 3>>("storage.payload", std::get<libjaguar::Vector<int, 3>>(p.storage));
				break;
			case 10:
				ow.SetOrCreate<libjaguar::Vector<int, 4>>("storage.payload", std::get<libjaguar::Vector<int, 4>>(p.storage));
				break;
			case 11:
				ow.SetOrCreate<libjaguar::Vector<unsigned int, 2>>("storage.payload", std::get<libjaguar::Vector<unsigned int, 2>>(p.storage));
				break;
			case 12:
				ow.SetOrCreate<libjaguar::Vector<unsigned int, 3>>("storage.payload", std::get<libjaguar::Vector<unsigned int, 3>>(p.storage));
				break;
			case 13:
				ow.SetOrCreate<libjaguar::Vector<unsigned int, 4>>("storage.payload", std::get<libjaguar::Vector<unsigned int, 4>>(p.storage));
				break;
			case 14:
				ow.SetOrCreate<libjaguar::Matrix<float, 2, 2>>("storage.payload", std::get<libjaguar::Matrix<float, 2, 2>>(p.storage));
				break;
			case 15:
				ow.SetOrCreate<libjaguar::Matrix<float, 3, 3>>("storage.payload", std::get<libjaguar::Matrix<float, 3, 3>>(p.storage));
				break;
			case 16:
				ow.SetOrCreate<libjaguar::Matrix<float, 4, 4>>("storage.payload", std::get<libjaguar::Matrix<float, 4, 4>>(p.storage));
				break;
		}
	}

	Material _DecMaterial(libjaguar::Document::ObjReader& rd) {
		Material mat = {};
		mat.shaderAddress = rd.Query<std::string>("shader");
		mat.parameters = rd.Query<std::vector<Material::Param>>("params");
		uint8_t tval = rd.Query<uint8_t>("transparency");
		CheckException(tval >= 0 && tval <= 2, "Invalid transparency value!");
		mat.transparency = static_cast<Material::TransparencyMode>(tval);
		return mat;
	}

	void _EncMaterial(const Material& m, libjaguar::Document::ObjWriter& ow) {
		ow.SetOrCreate<std::string>("shader", m.shaderAddress);
		ow.SetOrCreate<std::vector<Material::Param>>("params", m.parameters);
		ow.SetOrCreate<uint8_t>("transparency", static_cast<uint8_t>(m.transparency));
	}

	void _RegisterMaterialTypes(libjaguar::Document& doc) {
		//Tex ref
		libjaguar::StructuredTypeLayout rLayout = {};
		{
			libjaguar::StructuredTypeLayout::Field& addr = rLayout.fields.emplace_back();
			addr.name = "addr";
			addr.type = libjaguar::TypeTag::String;
		}
		{
			libjaguar::StructuredTypeLayout::Field& cube = rLayout.fields.emplace_back();
			cube.name = "cmap";
			cube.type = libjaguar::TypeTag::Boolean;
		}

		//Param
		libjaguar::StructuredTypeLayout pLayout = {};
		{
			libjaguar::StructuredTypeLayout::Field& tgt = pLayout.fields.emplace_back();
			tgt.name = "target";
			tgt.type = libjaguar::TypeTag::String;
		}
		{
			libjaguar::StructuredTypeLayout::Field& store = pLayout.fields.emplace_back();
			store.name = "storage";
			store.type = libjaguar::TypeTag::UnstructuredObj;
		}

		//Material itself
		libjaguar::StructuredTypeLayout mLayout = {};
		{
			libjaguar::StructuredTypeLayout::Field& shader = mLayout.fields.emplace_back();
			shader.name = "shader";
			shader.type = libjaguar::TypeTag::String;
		}
		{
			libjaguar::StructuredTypeLayout::Field& params = mLayout.fields.emplace_back();
			params.name = "params";
			params.type = libjaguar::TypeTag::List;
			params.elementType = libjaguar::TypeTag::StructuredObj;
			params.typeID = "MatParam";
		}
		{
			libjaguar::StructuredTypeLayout::Field& tmode = mLayout.fields.emplace_back();
			tmode.name = "transparency";
			tmode.type = libjaguar::TypeTag::UInt8;
		}

		//Register types
		doc.RegisterStructuredObjConverter<Material::TexRef>("MatTexRef", rLayout, _DecTexRef, _EncTexRef);
		doc.RegisterStructuredObjConverter<Material::Param>("MatParam", pLayout, _DecParam, _EncParam);
		doc.RegisterStructuredObjConverter<Material>("Material", mLayout, _DecMaterial, _EncMaterial);
	}

	Material DecodeMaterial(std::istream* stream) {
		CheckException(stream, "Invalid stream pointer!");
		CheckException(stream->good(), "Stream is broken!");

		//Check for header
		std::array<char, 6> headerChk;
		stream->read(headerChk.data(), headerChk.size());
		CheckException(stream->good(), "Failed to read material header!");
		CheckException(headerChk[0] == 'c' && headerChk[1] == 'e' && headerChk[2] == 'm' && headerChk[3] == 'a' && headerChk[4] == 't' && headerChk[5] == 'l', "Invalid material header!");

		//Check file revision
		uint16_t revision = 0;
		revision |= stream->get();
		CheckException(stream->good(), "Failed to read material version stamp!");
		revision |= (stream->get() << 8);
		CheckException(stream->good(), "Failed to read material version stamp!");
		CheckException(revision <= MAT_REVISION, "Material is of incompatible revision!");

		//Make objects
		std::unique_ptr<std::istream> ptr(stream);
		libjaguar::Document doc(std::move(ptr));
		_RegisterMaterialTypes(doc);

		//Check document
		libjaguar::ScopeEntry rootInfo = doc.QueryScopeInfo("");
		CheckException(rootInfo.subscopes.size() == 1 && rootInfo.subvalues.size() == 0, "Incorrect field count!");

		//Return result
		return doc.QueryValue<Material>("__MATERIAL__");
	}

	void EncodeMaterial(const Material& material, std::ostream* stream) {
		CheckException(stream, "Invalid stream pointer!");
		CheckException(stream->good(), "Stream is broken!");

		//Write header
		stream->write("cematl", 6);
		stream->put(MAT_REVISION & 0xF);
		stream->put(MAT_REVISION >> 8);

		//Create output document
		libjaguar::Document doc;
		_RegisterMaterialTypes(doc);

		//Write each field into doc
		doc.SetOrCreateValue<Material>("__MATERIAL__", material);

		//Export
		doc.ExportTo(*stream);
	}
}