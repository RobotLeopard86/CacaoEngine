#include "libcacaoasset.hpp"
#include "libcacaocommon.hpp"

#include "libjaguar/Document.hpp"

namespace libcacaoasset {
	Cubemap _DecCubemap(libjaguar::Document::ObjReader& rd) {
		Cubemap cmap = {};
		cmap.front = rd.Get<std::vector<unsigned char>>("front");
		cmap.back = rd.Get<std::vector<unsigned char>>("back");
		cmap.top = rd.Get<std::vector<unsigned char>>("top");
		cmap.bottom = rd.Get<std::vector<unsigned char>>("bottom");
		cmap.left = rd.Get<std::vector<unsigned char>>("left");
		cmap.right = rd.Get<std::vector<unsigned char>>("right");
		return cmap;
	}

	void _EncCubemap(const Cubemap& c, libjaguar::Document::ObjWriter& ow) {
		ow.SetOrCreate<std::vector<unsigned char>>("front", false, c.front);
		ow.SetOrCreate<std::vector<unsigned char>>("back", false, c.back);
		ow.SetOrCreate<std::vector<unsigned char>>("top", false, c.top);
		ow.SetOrCreate<std::vector<unsigned char>>("bottom", false, c.bottom);
		ow.SetOrCreate<std::vector<unsigned char>>("left", false, c.left);
		ow.SetOrCreate<std::vector<unsigned char>>("right", false, c.right);
	}

	void _RegisterCubemapTypes(libjaguar::Document& doc) {
		libjaguar::StructuredTypeLayout cmLayout = {};
		{
			libjaguar::StructuredTypeLayout::Field& f = cmLayout.fields.emplace_back();
			f.name = "front";
			f.type = libjaguar::TypeTag::ByteBuffer;
		}
		{
			libjaguar::StructuredTypeLayout::Field& b = cmLayout.fields.emplace_back();
			b.name = "back";
			b.type = libjaguar::TypeTag::ByteBuffer;
		}
		{
			libjaguar::StructuredTypeLayout::Field& t = cmLayout.fields.emplace_back();
			t.name = "top";
			t.type = libjaguar::TypeTag::ByteBuffer;
		}
		{
			libjaguar::StructuredTypeLayout::Field& b = cmLayout.fields.emplace_back();
			b.name = "bottom";
			b.type = libjaguar::TypeTag::ByteBuffer;
		}
		{
			libjaguar::StructuredTypeLayout::Field& l = cmLayout.fields.emplace_back();
			l.name = "left";
			l.type = libjaguar::TypeTag::ByteBuffer;
		}
		{
			libjaguar::StructuredTypeLayout::Field& r = cmLayout.fields.emplace_back();
			r.name = "right";
			r.type = libjaguar::TypeTag::ByteBuffer;
		}
		doc.RegisterStructuredObjConverter<Cubemap>("Cubemap", cmLayout, _DecCubemap, _EncCubemap);
	}

	Cubemap DecodeCubemap(std::istream* stream) {
		CheckException(stream, "Invalid stream pointer!");
		CheckException(stream->good(), "Stream is broken!");

		//Make objects
		std::unique_ptr<std::istream> ptr(stream);
		libjaguar::Document doc(std::move(ptr));
		_RegisterCubemapTypes(doc);

		//Check document
		libjaguar::ScopeEntry rootInfo = doc.QueryScopeInfo("");
		CheckException(rootInfo.subscopes.size() == 1 && rootInfo.subvalues.size() == 0, "Incorrect field count!");

		//Return result
		return doc.QueryValue<Cubemap>("__CMAP__");
	}

	void EncodeCubemap(const Cubemap& cubemap, std::ostream* stream) {
		CheckException(stream, "Invalid stream pointer!");
		CheckException(stream->good(), "Stream is broken!");

		//Create output document
		libjaguar::Document doc;
		_RegisterCubemapTypes(doc);

		//Write each field into doc
		doc.SetOrCreateValue<Cubemap>("__CMAP__", cubemap);

		//Export
		doc.ExportTo(*stream);
	}
}