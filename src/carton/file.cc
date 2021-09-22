#include "file.h"

#include <fstream>
#include <zstr.hpp>

#include "carton.h"
#include "metadata.h"

carton::File::File(carton::Carton* carton) {
	this->carton = carton;
	this->metadata = Metadata(carton);
}

void carton::File::setFileName(string fileName) {
	this->fileName = fileName;
	this->metadata.addMetadata("fileName", fileName);
}

string carton::File::getFileName() {
	return this->fileName;
}

void carton::File::write() {
	this->metadata.write();

	this->carton->writeEgg(Egg {
		type: FILE,
		continuedBlock: 0,
		compressionType: 0,
	});

	ifstream file(this->fileName);
	zstr::ostream compressed(this->carton->file);
	
	size_t bufferSize = 1 << 24;
	char* buffer = new char[bufferSize];
	size_t readCount = 0;
	do {
		file.read(buffer, bufferSize);
		readCount = file.gcount();
		compressed.write(buffer, readCount);
	}
	while(readCount > 0);
	delete buffer;

	file.close();
}
