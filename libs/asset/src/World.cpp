#include "libcacaoasset.hpp"
#include "libcacaocommon.hpp"

#include "libjaguar/Document.hpp"

namespace libcacaoasset {
	World DecodeWorld(std::istream* stream) {
		CheckException(stream, "Invalid stream pointer!");
		CheckException(stream->good(), "Stream is broken!");

		//TODO: Read
	}

	void EncodeWorld(const World& world, std::ostream* stream) {
		CheckException(stream, "Invalid stream pointer!");
		CheckException(stream->good(), "Stream is broken!");

		//Create output document
		libjaguar::Document doc;

		//TODO: Write

		//Export
		doc.ExportTo(*stream);
	}
}