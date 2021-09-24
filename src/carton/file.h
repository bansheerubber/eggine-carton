#pragma once

#include <string>

#include "egg.h"
#include "metadata.h"

using namespace std;

namespace carton {
	class File: public EggContents {
		friend class Carton;
		
		public:
			using EggContents::EggContents;
			File(class Carton* carton);
			File(class Carton* carton, Metadata* metadata);
			~File();
			
			Metadata* metadata = nullptr;
			void setFileName(string fileName);
			string getFileName();
		
		private:
			string fileName;
			bool compress = false;
			char* contents = nullptr; // contents of the file in memory

			void write();
			void read(Egg &header, unsigned int size);
	};
};
