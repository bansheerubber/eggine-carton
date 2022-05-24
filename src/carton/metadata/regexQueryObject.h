#pragma once

#include <regex>
#include <string>

#include "queryObject.h"

namespace carton {
	namespace metadata {
		class RegexQueryObject: public QueryObject {
			public:
				RegexQueryObject(class QueryList* list, std::string key, std::regex value);
				bool test(carton::Metadata* metadata);
			
			private:
				std::string key;
				std::regex value;
		};
	};
};
