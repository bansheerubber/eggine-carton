#pragma once

#include <string>

#include "queryObject.h"

using namespace std;

namespace carton {
	namespace metadata {
		class HasQueryObject: public QueryObject {
			public:
				HasQueryObject(class QueryList* list, string key);
				bool test(carton::Metadata* metadata);
			
			private:
				string key;
		};
	};
};
