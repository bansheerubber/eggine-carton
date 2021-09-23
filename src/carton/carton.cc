#include "carton.h"

#include "file.h"

carton::Carton::Carton() {
	
}

void carton::Carton::write(string fileName) {
	this->file.open(fileName, ios_base::out);

	const char* magic = "CARTON";
	this->file.write(magic, 6);

	this->stringTable.write();

	// write files
	for(File* file: this->files) {
		file->write();
	}

	this->file.flush();
	this->file.close();
}

void carton::Carton::read(string fileName) {
	this->file.open(fileName, ios_base::in);

	this->file.seekg(0, this->file.end);
	streampos totalSize = this->file.tellg();
	this->file.seekg(0, this->file.beg);

	char magic[6];
	this->file.read(magic, 6);

	if(string(magic) != string("CARTON")) {
		printf("could not read '%s'\n", fileName.c_str());
		exit(1);
	}

	Egg stringTableEgg = this->readEgg();
	if(stringTableEgg.type != STRING_TABLE) {
		printf("expected first egg to be string table for '%s'\n", fileName.c_str());
		exit(1);
	}

	this->stringTable.read(stringTableEgg); // first block must always be the string table

	Metadata* lastMetadata = nullptr;
	while(this->file.tellg() < totalSize) {
		Egg egg = this->readEgg();
		switch(egg.type) {
			case METADATA: {
				lastMetadata = new Metadata(this);
				lastMetadata->read(egg);
				break;
			}
			
			case FILE: {
				(new File(this, lastMetadata))->read(egg);
				break;
			}

			default: {
				printf("unexpected egg type '%d'\n", egg.type);
				exit(1);
			}
		}
	}

	this->file.close();
}

void carton::Carton::addFile(File* file) {
	this->files.push_back(file);
}

// write block header
streampos carton::Carton::writeEgg(Egg egg) {
	streampos start = this->file.tellp();
	
	this->writeNumber(egg.type);
	this->writeNumber(egg.blockSize);
	this->writeNumber(egg.continuedBlock);
	this->writeNumber(egg.compressionType);
	return start;
}

void carton::Carton::writeEggSize(unsigned int size, streampos position) {
	streampos currentPosition = this->file.tellp();
	this->file.seekp(position + sizeof(unsigned short int));
	this->writeNumber(size);
	this->file.seekp(currentPosition);
}

carton::Egg carton::Carton::readEgg() {
	return Egg {
		type: this->readNumber<unsigned short int>(),
		blockSize: this->readNumber<unsigned int>(),
		continuedBlock: this->readNumber<unsigned long>(),
		compressionType: this->readNumber<unsigned short int>(),
	};
}
