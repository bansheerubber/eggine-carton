#pragma once

#include <string>

#include "queryObject.h"

using namespace std;

namespace carton {
	namespace metadata {
		class EqualsQueryObject: public QueryObject {
			public:
				EqualsQueryObject(class QueryList* list, string key, string value);
				bool test(carton::Metadata* metadata);
			
			private:
				string key;
				string value;
		};
	};
};
