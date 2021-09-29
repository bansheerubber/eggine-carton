#pragma once

#include <string>
#include <vector>

#include "../../util/dynamicArray.h"

using namespace std;

namespace carton {
	class Metadata;
	class MetadataDatabase;
	
	namespace metadata {
		class QueryList {
			public:
				QueryList(class MetadataDatabase* database);
				~QueryList();
				
				QueryList* equals(string key, string test);
				QueryList* has(string key);
				DynamicArray<Metadata*> exec(bool debug = false); // dynamic array utility class has faster insert than std::vector
			
			private:
				MetadataDatabase* database;
				vector<class QueryObject*> objects;
		};
	};
};