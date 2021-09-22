#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <zstr.hpp>

#include "stringTable.h"

using namespace std;

namespace carton {
	enum EggTypes {
		INVALID_EGG,
		STRING_TABLE,
		METADATA,
		FILE,
	};

	enum EggCompressionTypes {
		NO_COMPRESSION,
	};
	
	struct Egg { // an egg is a block of data
		unsigned short int type;

		// if this block is separated into different blocks, then this specifies where its continued
		// bit 0 is used to determine if this block is a continued one or not
		unsigned long continuedBlock;
		unsigned short int compressionType;
	};
	
	class Carton {
		friend class File;
		friend class Metadata;
		friend StringTable;
		
		public:
			Carton();

			void write(string fileName);
			void addFile(class File* file);
		
		private:
			ofstream file;
			StringTable stringTable = StringTable(this);
			vector<class File*> files;
			
			void writeBytesLittleEndian(void* data, size_t size);

			void writeEgg(Egg egg);
			void writeData(unsigned char data);
			void writeData(unsigned short int data);
			void writeData(unsigned long data);
			void writeData(const char* data, unsigned char size);
			void writeData(const char* data, unsigned short size);
			void concatData(ifstream &stream);
			void concatData(zstr::ostream &stream);
	};
};
