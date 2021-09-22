#pragma once

#include <tsl/robin_map.h>
#include <string>

using namespace std;

typedef unsigned short metadata_value_length;

namespace carton {
	class Metadata { // metadata for a file block
		friend class Carton;
		friend class File;
		
		public:
			Metadata() {};
			Metadata(class Carton*);

			void addMetadata(string key, string value);

		private:
			class Carton* carton = nullptr;
			tsl::robin_map<string, string> metadata;

			void write();
	};
};
