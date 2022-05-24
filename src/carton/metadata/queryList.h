#pragma once

#include <string>
#include <vector>

#include "../../util/dynamicArray.h"

namespace carton {
	class Metadata;
	class MetadataDatabase;
	
	namespace metadata {
		class QueryList {
			public:
				QueryList(class MetadataDatabase* database);
				~QueryList();
				
				QueryList* equals(std::string key, std::string test);
				QueryList* has(std::string key);
				DynamicArray<Metadata*> exec(bool debug = false); // dynamic array utility class has faster insert than std::vector
			
			private:
				MetadataDatabase* database;
				std::vector<class QueryObject*> objects;
		};
	};
};