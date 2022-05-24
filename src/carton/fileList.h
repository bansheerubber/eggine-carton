#pragma once

#include <tsl/robin_map.h>
#include <string>

#include "egg.h"

typedef unsigned short metadata_value_length;

namespace carton {
	class FileList: public EggContents { // metadata for a file block
		friend class Carton;
		
		public:
			using EggContents::EggContents;

			void addFile(uint64_t position, std::string fileName);
			uint64_t getFile(std::string fileName);

		private:
			tsl::robin_map<std::string, uint64_t> filePositions; // start of the metadata + file combo
			tsl::robin_map<std::string, uint64_t> trueFilePositions; // start of the file, without the metadata

			void write();
			void read(Egg &header, unsigned int size);
	};
};
