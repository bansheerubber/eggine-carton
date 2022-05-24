#pragma once

#include <string>

#include "queryObject.h"

namespace carton {
	namespace metadata {
		class HasQueryObject: public QueryObject {
			public:
				HasQueryObject(class QueryList* list, std::string key);
				bool test(carton::Metadata* metadata);
			
			private:
				std::string key;
		};
	};
};
