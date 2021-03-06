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

carton::metadata::QueryList* carton::metadata::QueryList::equals(std::string key, std::string test) {
	this->objects.push_back(new EqualsQueryObject(this, key, test));
	return this;
}

carton::metadata::QueryList* carton::metadata::QueryList::has(std::string key) {
	this->objects.push_back(new HasQueryObject(this, key));
	return this;
}

DynamicArray<carton::Metadata*> carton::metadata::QueryList::exec(bool debug) {
	DynamicArray<carton::Metadata*> output(16);

	uint64_t start = std::chrono::duration_cast<std::chrono::microseconds>(
		std::chrono::high_resolution_clock::now().time_since_epoch()
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
		uint64_t duration = std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::high_resolution_clock::now().time_since_epoch()
		).count() - start;

		printf("Query Results (%lluus):\n", (unsigned long long)duration);
		for(uint64_t i = 0; i < output.head; i++) {
			printf("  - filename: %s\n", output[i]->getMetadata("fileName").c_str());
		}
	}

	delete this;

	return output;
}
