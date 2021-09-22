#include "metadata.h"

#include "carton.h"

carton::Metadata::Metadata(carton::Carton* carton) {
	this->carton = carton;
}

void carton::Metadata::addMetadata(string key, string value) {
	this->metadata[key] = value;
	this->carton->stringTable.addString(key);
}

void carton::Metadata::write() {
	this->carton->writeEgg(Egg {
		type: METADATA,
		continuedBlock: 0,
		compressionType: 0,
	});

	for(auto &[key, value]: this->metadata) {
		this->carton->writeData(this->carton->stringTable.lookup(key));
		this->carton->writeData(value.c_str(), (metadata_value_length)value.length());
	}
}
