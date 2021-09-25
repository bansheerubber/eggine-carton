#include "equalsQueryObject.h"

#include "../metadata.h"
#include "queryList.h"

carton::metadata::EqualsQueryObject::EqualsQueryObject(QueryList* list, string key, string value) : QueryObject(list) {
	this->key = key;
	this->value = value;
}

bool carton::metadata::EqualsQueryObject::test(carton::Metadata* metadata) {
	return metadata->getMetadata(this->key) == this->value;
}
