#pragma once

#include <tsl/robin_map.h>
#include <string>

#include "egg.h"

using namespace std;

typedef unsigned short metadata_value_length;

namespace carton {
	class Metadata: public EggContents { // metadata for a file block
		friend class Carton;
		friend class File;
		
		public:
			using EggContents::EggContents;

			void addMetadata(string key, string value);
			string getMetadata(string key);

		private:
			tsl::robin_map<string, string> metadata;

			void write();
			void read(Egg &header, unsigned int size);
	};
};
