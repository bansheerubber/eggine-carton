#include "queryList.h"

#include "equalsQueryObject.h"
#include "../metadata.h"
#include "metadataDatabase.h"
#include "hasQueryObject.h"

carton::metadata::QueryList::QueryList(MetadataDatabase* database) {
	this->database = database;
}

carton::metadata::QueryList::~QueryList() {
	for(QueryObject* object: this->objects) {
		delete object;
	}
}

carton::metadata::QueryList* carton::metadata::QueryList::equals(string key, string test) {
	this->objects.push_back(new EqualsQueryObject(this, key, test));
	return this;
}

carton::metadata::QueryList* carton::metadata::QueryList::has(string key) {
	this->objects.push_back(new HasQueryObject(this, key));
	return this;
}

tsl::robin_set<carton::Metadata*> carton::metadata::QueryList::exec() {
	tsl::robin_set<carton::Metadata*> output;

	for(carton::Metadata* metadata: this->database->metadata) {
		bool total = true;
		for(QueryObject* object: this->objects) {
			total &= object->test(metadata);
		}

		if(total) {
			output.insert(metadata);
		}
	}

	delete this;

	return output;
}
