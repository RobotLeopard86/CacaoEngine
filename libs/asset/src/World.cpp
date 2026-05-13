#include "libcacaoasset.hpp"
#include "libcacaocommon.hpp"

#include "libjaguar/Document.hpp"
#include "libjaguar/StructuredTypeLayout.hpp"
#include "libjaguar/TypeTags.hpp"

namespace libcacaoasset {
	World::Component _DecComponent(libjaguar::Document::ObjReader& rd) {
	}

	World::Actor _DecActor(libjaguar::Document::ObjReader& rd) {
	}

	World _DecWorld(libjaguar::Document::ObjReader& rd) {
	}

	void _EncComponent(const World::Component& c, libjaguar::Document::ObjWriter& ow) {
	}

	void _EncActor(const World::Actor& a, libjaguar::Document::ObjWriter& ow) {
	}

	void _EncWorld(const World& w, libjaguar::Document::ObjWriter& ow) {
	}

	void _RegisterWorldTypes(libjaguar::Document& doc) {
		//Component
		libjaguar::StructuredTypeLayout cLayout = {};
		{
			libjaguar::StructuredTypeLayout::Field& tid = cLayout.fields.emplace_back();
			tid.name = "typeid";
			tid.type = libjaguar::TypeTag::String;
		}
		{
			libjaguar::StructuredTypeLayout::Field& rfl = cLayout.fields.emplace_back();
			rfl.name = "rfl";
			rfl.type = libjaguar::TypeTag::ByteBuffer;
		}

		//Actor
		libjaguar::StructuredTypeLayout aLayout = {};
		{
			libjaguar::StructuredTypeLayout::Field& name = aLayout.fields.emplace_back();
			name.name = "name";
			name.type = libjaguar::TypeTag::String;
		}
		{
			libjaguar::StructuredTypeLayout::Field& guid = aLayout.fields.emplace_back();
			guid.name = "guid";
			guid.type = libjaguar::TypeTag::ByteBuffer;
		}
		{
			libjaguar::StructuredTypeLayout::Field& pguid = aLayout.fields.emplace_back();
			pguid.name = "pguid";
			pguid.type = libjaguar::TypeTag::ByteBuffer;
		}
		{
			libjaguar::StructuredTypeLayout::Field& pos = aLayout.fields.emplace_back();
			pos.name = "initPos";
			pos.type = libjaguar::TypeTag::Vector;
			pos.elementType = libjaguar::TypeTag::Float32;
			pos.width = 3;
		}
		{
			libjaguar::StructuredTypeLayout::Field& rot = aLayout.fields.emplace_back();
			rot.name = "initRot";
			rot.type = libjaguar::TypeTag::Vector;
			rot.elementType = libjaguar::TypeTag::Float32;
			rot.width = 3;
		}
		{
			libjaguar::StructuredTypeLayout::Field& scale = aLayout.fields.emplace_back();
			scale.name = "initScl";
			scale.type = libjaguar::TypeTag::Vector;
			scale.elementType = libjaguar::TypeTag::Float32;
			scale.width = 3;
		}
		{
			libjaguar::StructuredTypeLayout::Field& comps = aLayout.fields.emplace_back();
			comps.name = "components";
			comps.type = libjaguar::TypeTag::List;
			comps.elementType = libjaguar::TypeTag::StructuredObj;
			comps.typeID = "Component";
		}

		//World
		libjaguar::StructuredTypeLayout wLayout = {};
		{
			libjaguar::StructuredTypeLayout::Field& pos = wLayout.fields.emplace_back();
			pos.name = "camPos";
			pos.type = libjaguar::TypeTag::Vector;
			pos.elementType = libjaguar::TypeTag::Float32;
			pos.width = 3;
		}
		{
			libjaguar::StructuredTypeLayout::Field& rot = wLayout.fields.emplace_back();
			rot.name = "camRot";
			rot.type = libjaguar::TypeTag::Vector;
			rot.elementType = libjaguar::TypeTag::Float32;
			rot.width = 3;
		}
		{
			libjaguar::StructuredTypeLayout::Field& sky = aLayout.fields.emplace_back();
			sky.name = "skybox";
			sky.type = libjaguar::TypeTag::String;
		}
		{
			libjaguar::StructuredTypeLayout::Field& actors = aLayout.fields.emplace_back();
			actors.name = "actors";
			actors.type = libjaguar::TypeTag::List;
			actors.elementType = libjaguar::TypeTag::StructuredObj;
			actors.typeID = "Actor";
		}

		//Register types
		doc.RegisterStructuredObjConverter<World::Component>("Component", cLayout, _DecComponent, _EncComponent);
		doc.RegisterStructuredObjConverter<World::Actor>("Actor", aLayout, _DecActor, _EncActor);
		doc.RegisterStructuredObjConverter<World>("World", wLayout, _DecWorld, _EncWorld);
	}

	World DecodeWorld(std::istream* stream) {
		CheckException(stream, "Invalid stream pointer!");
		CheckException(stream->good(), "Stream is broken!");

		//Make objects
		std::unique_ptr<std::istream> ptr(stream);
		libjaguar::Document doc(std::move(ptr));
		_RegisterWorldTypes(doc);

		//Check document
		libjaguar::ScopeEntry rootInfo = doc.QueryScopeInfo("");
		CheckException(rootInfo.subscopes.size() == 1 && rootInfo.subvalues.size() == 0, "Incorrect field count!");

		//Return result
		return doc.QueryValue<World>("__WORLD__");
	}

	void EncodeWorld(const World& world, std::ostream* stream) {
		CheckException(stream, "Invalid stream pointer!");
		CheckException(stream->good(), "Stream is broken!");

		//Create output document
		libjaguar::Document doc;
		_RegisterWorldTypes(doc);

		//Write root field
		doc.SetOrCreateValue<World>("__WORLD__", world);

		//Export
		doc.ExportTo(*stream);
	}
}