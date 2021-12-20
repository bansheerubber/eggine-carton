#include "metadataDatabase.h"

#include "../carton.h"
#include "../metadata.h"
#include "queryList.h"

carton::MetadataDatabase::MetadataDatabase(Carton* carton) {
	this->carton = carton;
}

carton::MetadataDatabase::~MetadataDatabase() {
	for(Metadata* metadata: this->metadata) {
		delete metadata;
	}
}

void carton::MetadataDatabase::addMetadata(Metadata* metadata) {
	this->metadata.push_back(metadata);
}

carton::metadata::QueryList* carton::MetadataDatabase::get() {
	return new metadata::QueryList(this);
}
