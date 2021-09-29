#pragma once

#include <regex>
#include <string>

#include "queryObject.h"

using namespace std;

namespace carton {
	namespace metadata {
		class RegexQueryObject: public QueryObject {
			public:
				RegexQueryObject(class QueryList* list, string key, regex value);
				bool test(carton::Metadata* metadata);
			
			private:
				string key;
				regex value;
		};
	};
};
