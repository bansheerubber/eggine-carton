#include "queryList.h"

#include <chrono>

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

DynamicArray<carton::Metadata*> carton::metadata::QueryList::exec() {
	DynamicArray<carton::Metadata*> output(nullptr, 16, nullptr, nullptr);

	const bool debug = true;

	unsigned long long start = chrono::duration_cast<chrono::microseconds>(
		chrono::high_resolution_clock::now().time_since_epoch()
	).count();

	for(carton::Metadata* metadata: this->database->metadata) {
		bool total = true;
		for(QueryObject* object: this->objects) {
			total &= object->test(metadata);
			if(!total) {
				break;
			}
		}

		if(total) {
			output[output.head] = metadata;
			output.pushed();
		}
	}

	if(debug) {
		printf("query time: %lldus\n", chrono::duration_cast<chrono::microseconds>(
			chrono::high_resolution_clock::now().time_since_epoch()
		).count() - start);
	}

	delete this;

	return output;
}
