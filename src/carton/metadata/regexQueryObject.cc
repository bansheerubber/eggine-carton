#include "regexQueryObject.h"

#include "../metadata.h"
#include "queryList.h"

carton::metadata::RegexQueryObject::RegexQueryObject(QueryList* list, string key, regex value) : QueryObject(list) {
	this->key = key;
	this->value = value;
}

bool carton::metadata::RegexQueryObject::test(carton::Metadata* metadata) {
	return regex_search(metadata->getMetadata(this->key), this->value);
}
