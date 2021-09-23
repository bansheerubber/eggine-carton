#include "file.h"

#include <fstream>

#include "carton.h"
#include "metadata.h"

carton::File::File(Carton* carton) : EggContents(carton) {
	this->carton = carton;
	this->metadata = new Metadata(carton);
}

carton::File::File(Carton* carton, Metadata* metadata) : EggContents(carton) {
	this->carton = carton;
	this->metadata = metadata;
}

carton::File::~File() {
	if(this->metadata != nullptr) {
		delete this->metadata;
	}
}

void carton::File::setFileName(string fileName) {
	this->fileName = fileName;
	this->metadata->addMetadata("fileName", fileName);
}

string carton::File::getFileName() {
	return this->fileName;
}

void carton::File::write() {
	this->metadata->write();

	if(this->compress) {
		EggCompressionTypes level = ZLIB_LEVEL_6;
		
		streampos eggPosition = this->carton->writeEgg(Egg {
			type: FILE,
			blockSize: 0,
			continuedBlock: 0,
			compressionType: level,
		});

		ifstream file(this->fileName);
		size_t deflatedSize = this->carton->writeDeflated(file, level);
		file.close();

		this->carton->writeEggSize(deflatedSize, eggPosition);
	}
	else {
		streampos eggPosition = this->carton->writeEgg(Egg {
			type: FILE,
			blockSize: 0,
			continuedBlock: 0,
			compressionType: NO_COMPRESSION,
		});

		streampos start = this->carton->file.tellp();
		ifstream file(this->fileName);
		this->carton->file << file.rdbuf();
		file.close();
		this->carton->writeEggSize(this->carton->file.tellp() - start, eggPosition);
	}
}

void carton::File::read(Egg &header, unsigned int size) {
	this->setFileName(this->metadata->getMetadata("fileName"));
	
	if(header.compressionType == NO_COMPRESSION) { // if we have no compression, read from the file pointer
		this->carton->readFromFileIntoFileBuffer(size);
	}

	ofstream file("output/" + this->getFileName());
	file.write(this->carton->fileBuffer, this->carton->fileBufferSize);
	file.close();
}
