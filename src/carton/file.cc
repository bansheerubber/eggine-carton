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
		const size_t inBufferSize = 1 << 20; // read a megabyte at a time
		char inBuffer[inBufferSize];

		ofstream file("output/" + this->getFileName());
		streampos start = this->carton->file.tellg();
		size_t readBytes = 0;
		while(readBytes != header.blockSize) {
			this->carton->file.read(inBuffer, min(header.blockSize - readBytes, inBufferSize));
			size_t read = this->carton->file.gcount();
			readBytes += read;

			file.write(inBuffer, read);
		}

		file.close();
	}
	else { // if we have compression, read from the file buffer
		ofstream file("output/" + this->getFileName());
		file.write(this->carton->fileBuffer, this->carton->fileBufferSize);
		file.close();
	}
}
