#pragma once

#include <tsl/robin_map.h>
#include <string>

#include "egg.h"

typedef unsigned short metadata_value_length;

namespace carton {
	class Metadata: public EggContents { // metadata for a file block
		friend class Carton;
		friend class File;
		
		public:
			Metadata(Carton* carton);

			void addMetadata(std::string key, std::string value);
			std::string getMetadata(std::string key);
			bool hasMetadata(std::string key);
			void loadFromFile(std::string fileName);

		private:
			tsl::robin_map<std::string, std::string> metadata;

			uint64_t position;

			class File* owner = nullptr;

			void write();
			void read(Egg &header, unsigned int size);
	};
};
