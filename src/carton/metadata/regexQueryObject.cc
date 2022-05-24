#include "regexQueryObject.h"

#include "../metadata.h"
#include "queryList.h"

carton::metadata::RegexQueryObject::RegexQueryObject(QueryList* list, std::string key, std::regex value) : QueryObject(list) {
	this->key = key;
	this->value = value;
}

bool carton::metadata::RegexQueryObject::test(carton::Metadata* metadata) {
	return std::regex_search(metadata->getMetadata(this->key), this->value);
}
