#pragma once

#include <string>

#include "egg.h"
#include "metadata.h"

namespace carton {
	class File: public EggContents {
		friend class Carton;
		
		public:
			using EggContents::EggContents;
			File(class Carton* carton);
			File(class Carton* carton, Metadata* metadata);
			~File();
			
			Metadata* metadata = nullptr;
			void setFileName(std::string fileName);
			std::string getFileName();

			void saveToCachedBuffer(); // prevent the file from deleting itself after reading its contents and sending them through extension handler
		
		private:
			std::string fileName;
			bool compress = true;
			bool shouldDeleteAfterRead = true;
			bool deleteBuffer = true;
			char* contents = nullptr; // contents of the file in memory

			void write();
			void read(Egg &header, unsigned int size);
	};
};
