#include "metadata.h"

#include "carton.h"

void carton::Metadata::addMetadata(string key, string value) {
	this->metadata[key] = value;
	this->carton->stringTable.addString(key);
}

string carton::Metadata::getMetadata(string key) {
	return this->metadata[key];
}

void carton::Metadata::write() {
	streampos eggPosition = this->carton->writeEgg(Egg {
		type: METADATA,
		blockSize: 0,
		continuedBlock: 0,
		compressionType: 0,
	});

	unsigned int totalSize = 0;
	for(auto &[key, value]: this->metadata) {
		totalSize += this->carton->writeNumber(this->carton->stringTable.lookup(key));
		totalSize += this->carton->writeString(value.c_str(), (metadata_value_length)value.length());
	}
	
	this->carton->writeEggSize(totalSize, eggPosition);
}

void carton::Metadata::read(Egg &header) {
	streampos start = this->carton->file.tellg();
	while(this->carton->file.tellg() < start + header.blockSize) {
		string_table_index index = this->carton->readNumber<string_table_index>();
		this->addMetadata(this->carton->stringTable.lookup(index), this->carton->readString<metadata_value_length>());
	}
}
