#pragma once

#include <string>
#include <vector>

#include "equalsQueryObject.h"
#include "queryObject.h"

using namespace std;

namespace carton {
	namespace metadata {
		class QueryList;
	};
	
	// loads all metadata key/value pairs, and makes it possible to query them and return sets of metadata + filenames
	class MetadataDatabase {
		friend class Carton;
		friend metadata::QueryList;

		public:
			MetadataDatabase(class Carton* carton);

			void addMetadata(class Metadata* metadata);
			metadata::QueryList* get();

		private:
			vector<Metadata*> metadata;
			class Carton* carton;
	};
};
