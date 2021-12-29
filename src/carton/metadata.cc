#include "metadata.h"

#include <filesystem>

#include "carton.h"
#include "file.h"
#include "../util/trim.h"

carton::Metadata::Metadata(Carton* carton) : EggContents(carton) {
	carton->database.addMetadata(this);
}

void carton::Metadata::addMetadata(string key, string value) {
	this->metadata[key] = value;
	this->carton->stringTable.addString(key);
}

string carton::Metadata::getMetadata(string key) {
	auto it = this->metadata.find(key);
	if(it == this->metadata.end()) {
		return "";
	}
	else {
		return it.value();
	}
}

bool carton::Metadata::hasMetadata(string key) {
	return this->metadata.find(key) != this->metadata.end();
}

void carton::Metadata::loadFromFile(string fileName) {
	if(!filesystem::exists(fileName)) {
		return;
	}

	ifstream file(fileName);

	for(string line; getline(file, line);) {
		if(trim(line).length() == 0) {
			continue;
		}

		size_t equalsPosition = line.find("=");
		string key = trim(line.substr(0, equalsPosition));
		string value = trim(line.substr(equalsPosition + 1, line.length() - equalsPosition));
		this->addMetadata(key, value);
	}

	file.close();
}

void carton::Metadata::write() {
	uint64_t eggPosition = this->carton->writeEgg(Egg {
		type: METADATA,
		blockSize: 0,
		continuedBlock: 0,
		compressionType: 0,
	});

	this->position = eggPosition;

	unsigned int totalSize = 0;
	for(auto &[key, value]: this->metadata) {
		totalSize += this->carton->writeNumber(this->carton->stringTable.lookup(key));
		totalSize += this->carton->writeString(value.c_str(), (metadata_value_length)value.length());
	}
	
	this->carton->writeEggSize(totalSize, eggPosition);
}

void carton::Metadata::read(Egg &header, unsigned int size) {
	uint64_t start = (uint64_t)this->carton->file.tellg();
	while(this->carton->canRead(start, size)) {
		string_table_index index = this->carton->readNumber<string_table_index>();
		this->addMetadata(this->carton->stringTable.lookup(index), this->carton->readString<metadata_value_length>());
	}
}
