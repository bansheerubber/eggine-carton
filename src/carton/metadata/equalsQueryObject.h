#pragma once

#include <string>

#include "queryObject.h"

namespace carton {
	namespace metadata {
		class EqualsQueryObject: public QueryObject {
			public:
				EqualsQueryObject(class QueryList* list, std::string key, std::string value);
				bool test(carton::Metadata* metadata);
			
			private:
				std::string key;
				std::string value;
		};
	};
};
