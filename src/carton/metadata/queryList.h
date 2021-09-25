#pragma once

#include <tsl/robin_set.h>
#include <string>
#include <vector>

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
				tsl::robin_set<Metadata*> exec();
			
			private:
				MetadataDatabase* database;
				vector<class QueryObject*> objects;
		};
	};
};