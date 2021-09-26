#pragma once

#include <tsl/robin_map.h>
#include <string>

#include "egg.h"

using namespace std;

typedef unsigned short metadata_value_length;

namespace carton {
	class FileList: public EggContents { // metadata for a file block
		friend class Carton;
		
		public:
			using EggContents::EggContents;

			void addFile(streampos position, string fileName);

		private:
			tsl::robin_map<string, streampos> filePositions;

			void write();
			void read(Egg &header, unsigned int size);
	};
};
