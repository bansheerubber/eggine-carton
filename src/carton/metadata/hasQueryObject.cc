#include "hasQueryObject.h"

#include "../metadata.h"
#include "queryList.h"

carton::metadata::HasQueryObject::HasQueryObject(QueryList* list, std::string key) : QueryObject(list) {
	this->key = key;
}

bool carton::metadata::HasQueryObject::test(carton::Metadata* metadata) {
	return metadata->hasMetadata(this->key);
}
