#include "libcacaoasset.hpp"
#include "libcacaocommon.hpp"

#include "libjaguar/Document.hpp"
#include "libjaguar/StructuredTypeLayout.hpp"
#include "libjaguar/TypeTags.hpp"

namespace libcacaoasset {
	Shader::Descriptor::UniformParameter _DecUniformParameter(libjaguar::Document::ObjReader& rd) {
	}

	Shader::Descriptor::TextureParameter _DecTextureParameter(libjaguar::Document::ObjReader& rd) {
	}

	Shader _DecShader(libjaguar::Document::ObjReader& rd) {
	}

	void _EncUniformParameter(const Shader::Descriptor::UniformParameter& u, libjaguar::Document::ObjWriter& ow) {
	}

	void _EncTextureParameter(const Shader::Descriptor::TextureParameter& t, libjaguar::Document::ObjWriter& ow) {
	}

	void _EncShader(const Shader& s, libjaguar::Document::ObjWriter& ow) {
	}

	void _RegisterShaderTypes(libjaguar::Document& doc) {
		//Uniform parameter
		libjaguar::StructuredTypeLayout uLayout = {};
		{
			libjaguar::StructuredTypeLayout::Field& type = uLayout.fields.emplace_back();
			type.name = "type";
			type.type = libjaguar::TypeTag::UInt8;
		}
		{
			libjaguar::StructuredTypeLayout::Field& off = uLayout.fields.emplace_back();
			off.name = "uboOff";
			off.type = libjaguar::TypeTag::UInt32;
		}
		{
			libjaguar::StructuredTypeLayout::Field& name = uLayout.fields.emplace_back();
			name.name = "name";
			name.type = libjaguar::TypeTag::String;
		}

		//Texture parameter
		libjaguar::StructuredTypeLayout tLayout = {};
		{
			libjaguar::StructuredTypeLayout::Field& meIsCube = tLayout.fields.emplace_back();
			meIsCube.name = "cube";
			meIsCube.type = libjaguar::TypeTag::Boolean;
		}
		{
			libjaguar::StructuredTypeLayout::Field& bind = tLayout.fields.emplace_back();
			bind.name = "binding";
			bind.type = libjaguar::TypeTag::UInt32;
		}
		{
			libjaguar::StructuredTypeLayout::Field& name = tLayout.fields.emplace_back();
			name.name = "name";
			name.type = libjaguar::TypeTag::String;
		}

		//Shader
		libjaguar::StructuredTypeLayout sLayout = {};
		{
			libjaguar::StructuredTypeLayout::Field& code = sLayout.fields.emplace_back();
			code.name = "code";
			code.type = libjaguar::TypeTag::ByteBuffer;
		}
		{
			libjaguar::StructuredTypeLayout::Field& in = sLayout.fields.emplace_back();
			in.name = "inbits";
			in.type = libjaguar::TypeTag::UInt8;
		}
		{
			libjaguar::StructuredTypeLayout::Field& type = sLayout.fields.emplace_back();
			type.name = "type";
			type.type = libjaguar::TypeTag::UInt8;
		}
		{
			libjaguar::StructuredTypeLayout::Field& uparam = sLayout.fields.emplace_back();
			uparam.name = "uparams";
			uparam.type = libjaguar::TypeTag::List;
			uparam.elementType = libjaguar::TypeTag::StructuredObj;
			uparam.typeID = "uprm";
		}
		{
			libjaguar::StructuredTypeLayout::Field& tparam = sLayout.fields.emplace_back();
			tparam.name = "tparams";
			tparam.type = libjaguar::TypeTag::List;
			tparam.elementType = libjaguar::TypeTag::StructuredObj;
			tparam.typeID = "tprm";
		}

		//Register types
		doc.RegisterStructuredObjConverter<Shader::Descriptor::UniformParameter>("uprm", uLayout, _DecUniformParameter, _EncUniformParameter);
		doc.RegisterStructuredObjConverter<Shader::Descriptor::TextureParameter>("tprm", tLayout, _DecTextureParameter, _EncTextureParameter);
		doc.RegisterStructuredObjConverter<Shader>("Shader", sLayout, _DecShader, _EncShader);
	}

	Shader DecodeShader(std::istream* stream) {
		CheckException(stream, "Invalid stream pointer!");
		CheckException(stream->good(), "Stream is broken!");

		//Make objects
		std::unique_ptr<std::istream> ptr(stream);
		libjaguar::Document doc(std::move(ptr));
		_RegisterShaderTypes(doc);

		//Check document
		libjaguar::ScopeEntry rootInfo = doc.QueryScopeInfo("");
		CheckException(rootInfo.subscopes.size() == 1 && rootInfo.subvalues.size() == 0, "Incorrect field count!");

		//Return result
		return doc.QueryValue<Shader>("__SHADER__");
	}

	void EncodeShader(const Shader& shader, std::ostream* stream) {
		CheckException(stream, "Invalid stream pointer!");
		CheckException(stream->good(), "Stream is broken!");

		//Create output document
		libjaguar::Document doc;
		_RegisterShaderTypes(doc);

		//Write fields
		doc.SetOrCreateValue<Shader>("__SHADER__", shader);

		//Export
		doc.ExportTo(*stream);
	}
}