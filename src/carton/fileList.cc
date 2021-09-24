#include "fileList.h"

#include "carton.h"

void carton::FileList::addFile(streampos position, string fileName) {
	this->filePositions[fileName] = position;
}

void carton::FileList::write() {
	streampos eggPosition = this->carton->writeEgg(Egg {
		type: FILE_LIST,
		blockSize: 0,
		continuedBlock: 0,
		compressionType: NO_COMPRESSION,
	});

	unsigned int totalSize = 0;
	for(auto &[key, value]: this->filePositions) {
		totalSize += this->carton->writeNumber((unsigned long)value);
		totalSize += this->carton->writeString(key.c_str(), (unsigned short)key.length()); // write string
	}

	this->carton->writeEggSize(totalSize, eggPosition);

	streampos position = this->carton->file.tellp();
	this->carton->file.seekp(this->carton->fileListPointerPosition);
	this->carton->writeNumber((unsigned long)eggPosition);
	this->carton->file.seekp(position);
}

void carton::FileList::read(Egg &header, unsigned int size) {
	streampos start = this->carton->file.tellg();
	while(this->carton->canRead(start, size)) {
		unsigned long position = this->carton->readNumber<unsigned long>();
		this->addFile(position, this->carton->readString<unsigned short>());
	}
}
