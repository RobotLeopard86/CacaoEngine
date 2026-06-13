#include "libcacaoasset.hpp"
#include "CheckException.hpp"

#include "libjaguar/Document.hpp"
#include "libjaguar/StructuredTypeLayout.hpp"
#include "libjaguar/TypeTags.hpp"

#include <vector>

#define SHADER_REVISION uint16_t(1)

namespace libcacaoasset {
	Shader::Descriptor::UniformParameter _DecUniformParameter(libjaguar::Document::ObjReader& rd) {
		Shader::Descriptor::UniformParameter uparam;
		uparam.name = rd.Query<std::string>("name");
		uint8_t typeByte = rd.Query<uint8_t>("type");
		CheckException(typeByte == 0x22 || typeByte == 0x33 || typeByte == 0x44 || typeByte == 0xB1 || ((typeByte >> 8) >= 0xD && (typeByte & 0xF) >= 1 && (typeByte & 0xF) <= 4), "Bad type byte!");
		uparam.type = static_cast<Shader::Descriptor::UniformParameter::DataType>(typeByte);
		uparam.bufferOffset = rd.Query<unsigned int>("uboOff");
		return uparam;
	}

	Shader::Descriptor::TextureParameter _DecTextureParameter(libjaguar::Document::ObjReader& rd) {
		Shader::Descriptor::TextureParameter tparam;
		tparam.name = rd.Query<std::string>("name");
		tparam.isCubemap = rd.Query<bool>("cube");
		tparam.binding = rd.Query<unsigned int>("binding");
		return tparam;
	}

	Shader _DecShader(libjaguar::Document::ObjReader& rd) {
		Shader shader;
		shader.irCode = rd.Query<std::vector<unsigned char>>("ir");
		shader.descriptor.vertexInputs = static_cast<Shader::Descriptor::VertexInputBits>(rd.Query<uint8_t>("vInputs"));
		shader.descriptor.objectInputs = static_cast<Shader::Descriptor::ObjectInputBits>(rd.Query<uint8_t>("oInputs"));
		uint8_t domainByte = rd.Query<uint8_t>("domain");
		CheckException(domainByte == 0xA || domainByte == 0xD || domainByte == 0xE, "Bad domain byte!");
		shader.descriptor.domain = static_cast<Shader::Descriptor::Domain>(domainByte);
		shader.descriptor.uniformParams = rd.Query<std::vector<Shader::Descriptor::UniformParameter>>("uparams");
		shader.descriptor.texParams = rd.Query<std::vector<Shader::Descriptor::TextureParameter>>("tparams");
		return shader;
	}

	void _EncUniformParameter(const Shader::Descriptor::UniformParameter& u, libjaguar::Document::ObjWriter& ow) {
		ow.SetOrCreate<std::string>("name", u.name);
		ow.SetOrCreate<unsigned int>("uboOff", u.bufferOffset);
		ow.SetOrCreate<uint8_t>("type", static_cast<uint8_t>(u.type));
	}

	void _EncTextureParameter(const Shader::Descriptor::TextureParameter& t, libjaguar::Document::ObjWriter& ow) {
		ow.SetOrCreate<std::string>("name", t.name);
		ow.SetOrCreate<unsigned int>("binding", t.binding);
		ow.SetOrCreate<bool>("cube", t.isCubemap);
	}

	void _EncShader(const Shader& s, libjaguar::Document::ObjWriter& ow) {
		ow.SetOrCreate<std::vector<unsigned char>>("ir", false, s.irCode);
		ow.SetOrCreate<uint8_t>("domain", static_cast<uint8_t>(s.descriptor.domain));
		ow.SetOrCreate<uint8_t>("vInputs", static_cast<uint8_t>(s.descriptor.vertexInputs));
		ow.SetOrCreate<uint8_t>("oInputs", static_cast<uint8_t>(s.descriptor.vertexInputs));
		ow.SetOrCreate<std::vector<Shader::Descriptor::UniformParameter>>("uparams", s.descriptor.uniformParams);
		ow.SetOrCreate<std::vector<Shader::Descriptor::TextureParameter>>("tparams", s.descriptor.texParams);
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
			libjaguar::StructuredTypeLayout::Field& ir = sLayout.fields.emplace_back();
			ir.name = "ir";
			ir.type = libjaguar::TypeTag::ByteBuffer;
		}
		{
			libjaguar::StructuredTypeLayout::Field& vIn = sLayout.fields.emplace_back();
			vIn.name = "vInputs";
			vIn.type = libjaguar::TypeTag::UInt8;
		}
		{
			libjaguar::StructuredTypeLayout::Field& oIn = sLayout.fields.emplace_back();
			oIn.name = "oInputs";
			oIn.type = libjaguar::TypeTag::UInt8;
		}
		{
			libjaguar::StructuredTypeLayout::Field& domain = sLayout.fields.emplace_back();
			domain.name = "domain";
			domain.type = libjaguar::TypeTag::UInt8;
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

		//Check for header
		std::array<char, 6> headerChk;
		stream->read(headerChk.data(), headerChk.size());
		CheckException(stream->good(), "Failed to read shader header!");
		CheckException(headerChk[0] == 'c' && headerChk[1] == 'e' && headerChk[2] == 's' && headerChk[3] == 'h' && headerChk[4] == 'd' && headerChk[5] == 'r', "Invalid shader header!");

		//Check file revision
		uint16_t revision = 0;
		revision |= stream->get();
		CheckException(stream->good(), "Failed to read shader version stamp!");
		revision |= (stream->get() << 8);
		CheckException(stream->good(), "Failed to read shader version stamp!");
		CheckException(revision <= SHADER_REVISION, "Shader is of incompatible revision!");

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

		//Write header
		stream->write("ceshdr", 6);
		stream->put(SHADER_REVISION & 0xF);
		stream->put(SHADER_REVISION >> 8);

		//Create output document
		libjaguar::Document doc;
		_RegisterShaderTypes(doc);

		//Write fields
		doc.SetOrCreateValue<Shader>("__SHADER__", shader);

		//Export
		doc.ExportTo(*stream);
	}
}