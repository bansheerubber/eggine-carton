#include "file.h"

#include <cstring>
#include <filesystem>
#include <fstream>

#include "carton.h"
#include "fileList.h"
#include "metadata.h"

carton::File::File(Carton* carton) : EggContents(carton) {
	this->carton = carton;
	this->metadata = new Metadata(carton);
	this->metadata->owner = this;
}

carton::File::File(Carton* carton, Metadata* metadata) : EggContents(carton) {
	this->carton = carton;
	this->metadata = metadata;
}

carton::File::~File() {
	if(this->contents != nullptr) {
		delete this->contents;
	}
}

void carton::File::setFileName(std::string fileName) {
	this->fileName = fileName;

	// remove packing directory if there is one
	if(this->carton->packingDirectory.length()) {
		fileName = fileName.replace(fileName.begin(), fileName.begin() + this->carton->packingDirectory.length() + 1, "");
	}
	this->metadata->addMetadata("fileName", fileName);
	this->metadata->addMetadata("extension", std::filesystem::path(fileName).extension().string());

	this->metadata->loadFromFile(this->fileName + ".metadata");
}

std::string carton::File::getFileName() {
	return this->fileName;
}

void carton::File::write() {
	this->metadata->write();

	std::string compress = this->metadata->getMetadata("compress");

	if(compress == "" || compress == "1" || compress == "true") {
		EggCompressionTypes level = ZLIB_LEVEL_6;
		
		uint64_t eggPosition = this->carton->writeEgg(Egg {
			type: FILE,
			blockSize: 0,
			continuedBlock: 0,
			compressionType: level,
		});
		this->carton->fileList.addFile(this->metadata->position, this->metadata->getMetadata("fileName"));

		std::ifstream file(this->fileName);
		uint64_t deflatedSize = this->carton->writeDeflated(file, level);
		file.close();

		this->carton->writeEggSize(deflatedSize, eggPosition);
	}
	else {
		uint64_t eggPosition = this->carton->writeEgg(Egg {
			type: FILE,
			blockSize: 0,
			continuedBlock: 0,
			compressionType: NO_COMPRESSION,
		});
		this->carton->fileList.addFile(this->metadata->position, this->metadata->getMetadata("fileName"));

		uint64_t start = (uint64_t)this->carton->file.tellp();
		std::ifstream file(this->fileName);

		file.seekg(0, file.end);
		uint64_t length = (uint64_t)file.tellg();
		file.seekg(0, file.beg);

		if(length) {
			this->carton->file << file.rdbuf();
		}
		
		file.close();
		this->carton->writeEggSize((uint64_t)this->carton->file.tellp() - start, eggPosition);
	}
}

void carton::File::read(Egg &header, unsigned int size) {
	this->setFileName(this->metadata->getMetadata("fileName"));

	if(header.compressionType == NO_COMPRESSION) { // if we have no compression, read from the file pointer
		this->carton->readFromFileIntoFileBuffer(size);
	}

	if(this->contents != nullptr) {
		delete this->contents;
	}

	auto it = this->carton->extensionHandlers.find(std::filesystem::path(this->getFileName()).extension().string());
	if(it != this->carton->extensionHandlers.end()) {
		(*it.value().first)(it.value().second, this, this->carton->fileBuffer, this->carton->fileBufferSize);
	}

	if(this->carton->shouldExport) {
		std::string output = this->carton->exportDirectory + "/" + this->fileName;
		std::filesystem::create_directories(std::filesystem::path(output).parent_path());
		std::ofstream file(output);
		file.write(this->carton->fileBuffer, this->carton->fileBufferSize);
		file.close();
	}

	if(this->shouldDeleteAfterRead) {
		delete this;
	}
	else {
		this->contents = new char[this->carton->fileBufferSize];
		memcpy(this->contents, this->carton->fileBuffer, this->carton->fileBufferSize); // copy into our content buffer
	}

	if(header.compressionType == NO_COMPRESSION && this->deleteBuffer) {
		this->carton->deleteFileBuffer();
	}
}

void carton::File::saveToCachedBuffer() {
	this->shouldDeleteAfterRead = false;
}
