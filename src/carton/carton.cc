#include "carton.h"

#include <cstring>
#include <fstream>
#include <zlib.h>

#include "file.h"
#include "../util/md5hash.h"

carton::Carton::Carton() {
	
}

carton::Carton::~Carton() {
	for(File* file: this->files) {
		delete file;
	}
}

void carton::Carton::write(std::string fileName) {
	this->file.open(fileName, std::ios_base::out | std::ios::binary);

	const char* magic = "CARTON";
	this->file.write(magic, 6);

	this->writeNumber(this->version);

	this->fileListPointerPosition = (uint64_t)this->file.tellp();
	uint64_t fileListPointer = 0;
	this->writeNumber(fileListPointer);

	this->stringTable.write();

	// write files
	for(File* file: this->files) {
		file->write();
	}

	this->fileList.write();

	this->file.flush();
	this->file.close();
}

void carton::Carton::read(std::string fileName) {
	#ifndef __switch__
	md5hash(fileName, this->hash.hash);
	#endif
	
	this->file.open(fileName, std::ios_base::in | std::ios::binary);

	this->file.seekg(0, this->file.end);
	this->totalSize = (uint64_t)this->file.tellg();
	this->file.seekg(0, this->file.beg);

	char magic[6];
	this->file.read(magic, 6);

	if(std::string(magic, 6) != std::string("CARTON")) {
		printf("could not read '%s'\n", fileName.c_str());
		exit(1);
	}

	unsigned int version = this->readNumber<unsigned int>();
	if(this->version != version) {
		printf("version mismatch: %u != %u\n", this->version, version);
		exit(1);
	}

	uint64_t fileListPointer = this->readNumber<uint64_t>();

	// read the string table
	if(&this->stringTable != this->parseEggContents()) {
		printf("expected string table for '%s'\n", fileName.c_str());
	}

	// read the file list
	this->file.seekg(fileListPointer);
	if(&this->fileList != this->parseEggContents()) {
		printf("expected file list for '%s'\n", fileName.c_str());
	}

	// loop through file list and read file metadata to build lookup tables
	for(auto &[fileName, position]: this->fileList.filePositions) {
		this->file.seekg(position);
		this->parseEggContents(); // read metadata
		this->fileList.trueFilePositions[fileName] = (uint64_t)this->file.tellg();
	}
}

void carton::Carton::addFile(File* file) {
	this->files.push_back(file);
}

carton::File* carton::Carton::readFile(std::string fileName) {
	this->file.seekg(this->fileList.getFile(fileName));
	this->parseEggContents(); // read metadata
	File* file = (File*)this->parseEggContents();
	return file;
}

carton::FileBuffer carton::Carton::readFileToBuffer(std::string fileName) {
	this->file.seekg(this->fileList.getFile(fileName));
	this->parseEggContents(); // read metadata
	this->parseEggContents(false); // read file
	FileBuffer result = {
		buffer: (const unsigned char*)this->fileBuffer,
		size: this->fileBufferSize,
	};

	this->fileBuffer = nullptr;
	this->fileBufferSize = 0;
	this->fileBufferPointer = 0;	

	return result;
}

uint64_t carton::Carton::getFileLocation(std::string fileName) {
	return this->fileList.trueFilePositions[fileName] + sizeof(Egg::type) + sizeof(Egg::blockSize) + sizeof(Egg::continuedBlock) + sizeof(Egg::compressionType);
}

uint64_t carton::Carton::getFileSize(std::string fileName) {
	this->file.seekg(this->fileList.trueFilePositions[fileName]);
	Egg egg = this->readEgg();
	return egg.blockSize;
}

void carton::Carton::exportFiles() {
	this->shouldExport = true;
	for(auto &[file, _]: this->fileList.filePositions) {
		this->readFile(file);
	}
}

void carton::Carton::addExtensionHandler(std::string extension, file_extension_handler handler, void* owner) {
	this->extensionHandlers[extension] = std::pair(handler, owner);
}

// write block header
uint64_t carton::Carton::writeEgg(Egg egg) {
	uint64_t start = (uint64_t)this->file.tellp();
	
	this->writeNumber(egg.type);
	this->writeNumber(egg.blockSize);
	this->writeNumber(egg.continuedBlock);
	this->writeNumber(egg.compressionType);
	return start;
}

void carton::Carton::writeEggSize(unsigned int size, uint64_t position) {
	uint64_t currentPosition = (uint64_t)this->file.tellp();
	this->file.seekp((uint64_t)position + sizeof(unsigned short int));
	this->writeNumber(size);
	this->file.seekp(currentPosition);
}

carton::Egg carton::Carton::readEgg() {
	return Egg {
		type: this->readNumber<unsigned short int>(),
		blockSize: this->readNumber<unsigned int>(),
		continuedBlock: this->readNumber<uint64_t>(),
		compressionType: this->readNumber<unsigned short int>(),
	};
}

carton::EggContents* carton::Carton::parseEggContents(bool deleteBuffer) {
	uint64_t start = (uint64_t)this->file.tellg();

	auto it = this->positionToContents.find(start);
	if(it != this->positionToContents.end()) {
		this->file.seekg(this->contentsToEnd[it.value()]);
		return it.value();
	}
	
	Egg egg = this->readEgg();
	EggContents* output = nullptr;

	unsigned int size = egg.blockSize;
	if(egg.compressionType != NO_COMPRESSION) {
		this->readInflatedIntoFileBuffer((EggCompressionTypes)egg.compressionType, egg.blockSize);
		this->fileBufferSize = this->fileBufferPointer;
		this->fileBufferPointer = 0;
		size = this->fileBufferSize;
	}

	switch(egg.type) {
		case STRING_TABLE: {
			this->stringTable.read(egg, size);
			output = &this->stringTable;
			break;
		}
		
		case METADATA: {
			output = new Metadata(this);
			output->read(egg, size);
			break;
		}
		
		case FILE: {
			output = new File(this, (Metadata*)this->endToContents[start]);
			((File*)output)->deleteBuffer = deleteBuffer;
			output->read(egg, size);
			break;
		}

		case FILE_LIST: {
			this->fileList.read(egg, size);
			output = &this->fileList;
			break;
		}

		default: {
			exit(1);
		}
	}

	if(this->contents.find(output) != this->contents.end()) { // make sure it wasn't deleted
		this->positionToContents[start] = output;
		this->contentsToEnd[output] = (uint64_t)this->file.tellg();
		this->endToContents[(uint64_t)this->file.tellg()] = output;
	}

	if(egg.compressionType != NO_COMPRESSION && deleteBuffer) {
		this->deleteFileBuffer(); // clean up the mess
	}

	return output;
}

void carton::Carton::setPackingDirectory(std::string packingDirectory) {
	this->packingDirectory = packingDirectory;
}

uint64_t carton::__writeDeflated(carton::Carton* carton, std::istream* input, const char* buffer, uint64_t bufferSize, carton::EggCompressionTypes level) {
	if(level < carton::ZLIB_LEVEL_0 || level > carton::ZLIB_LEVEL_9) {
		printf("invalid deflate level %d\n", level);
		exit(1);
	}
	
	const uint64_t outBufferSize = 1 << 20; // write a megabyte at a time
	unsigned char outBuffer[outBufferSize];

	const uint64_t inBufferSize = 1 << 20; // read a megabyte at a time
	char* inBuffer = nullptr;
	if(input != nullptr) {
		inBuffer = new char[inBufferSize];
	}

	z_stream stream {
		next_in: (unsigned char*)inBuffer,
		avail_in: inBufferSize,
		next_out: outBuffer,
		avail_out: outBufferSize,
		zalloc: 0,
		zfree: 0,
	};

	uint64_t start = (uint64_t)carton->file.tellp();
	deflateInit(&stream, level - carton::ZLIB_LEVEL_0);

	uint64_t totalRead = 0;
	while(true) {
		if(input != nullptr) {
			input->read(inBuffer, inBufferSize);
			std::streamsize readCount = input->gcount();
			
			stream.next_in = (unsigned char*)inBuffer;
			stream.avail_in = readCount;

			if(readCount == 0) {
				break;
			}
		}
		else {
			if(totalRead >= bufferSize) {
				break;
			}
			
			stream.next_in = (unsigned char*)&buffer[totalRead];
			stream.avail_in = std::min(bufferSize - totalRead, inBufferSize); // try to read a megabyte at a time

			totalRead += stream.avail_in;
		}

		// write the result to the file
		do {
			deflate(&stream, 0);
			carton->file.write((char*)outBuffer, outBufferSize - stream.avail_out);

			stream.next_out = outBuffer;
			stream.avail_out = outBufferSize;
		}
		while(stream.avail_in > 0);
	}

	int result;
	do {
		result = deflate(&stream, Z_FINISH);
		carton->file.write((char*)outBuffer, outBufferSize - stream.avail_out);

		stream.next_out = outBuffer;
		stream.avail_out = outBufferSize;
	}
	while(result == Z_OK);
	
	deflateEnd(&stream);
	delete inBuffer;

	return (uint64_t)carton->file.tellp() - start;
}

uint64_t carton::Carton::writeDeflated(std::istream &input, EggCompressionTypes level) {
	return __writeDeflated(this, &input, nullptr, 0, level);
}

uint64_t carton::Carton::writeDeflated(char* buffer, uint64_t size, EggCompressionTypes level) {
	return __writeDeflated(this, nullptr, buffer, size, level);
}

void carton::Carton::initFileBuffer() {
	this->deleteFileBuffer();

	this->fileBufferSize = 1024;
	this->fileBufferPointer = 0;
	this->fileBuffer = new char[this->fileBufferSize];
}

void carton::Carton::deleteFileBuffer() {
	if(this->fileBuffer != nullptr) {
		free(this->fileBuffer);
	}

	this->fileBuffer = nullptr;
	this->fileBufferSize = 0;
	this->fileBufferPointer = 0;
}

void carton::Carton::commitFileBuffer() {
	this->file.write(this->fileBuffer, this->fileBufferPointer);
	this->deleteFileBuffer();
}

uint64_t carton::Carton::commitDeflatedFileBuffer(EggCompressionTypes compression) {
	uint64_t size = this->writeDeflated(this->fileBuffer, this->fileBufferPointer, compression);
	this->deleteFileBuffer();
	return size;
}

void carton::Carton::writeToFileBuffer(char byte) {
	this->fileBuffer[this->fileBufferPointer] = byte;
	this->fileBufferPointer++;
	
	if(this->fileBufferPointer == this->fileBufferSize) { // resize buffer
		this->fileBufferSize = this->fileBufferSize * 2; // new size
		this->fileBuffer = (char*)realloc(this->fileBuffer, sizeof(char) * this->fileBufferSize);
	}
}

void carton::Carton::writeBytesToFileBuffer(char* bytes, uint64_t size) {
	uint64_t newSize = this->fileBufferPointer + size;
	while(this->fileBufferSize < newSize) {
		this->fileBufferSize = this->fileBufferSize * 2; // new size
		this->fileBuffer = (char*)realloc(this->fileBuffer, sizeof(char) * this->fileBufferSize);
	}

	memcpy(&this->fileBuffer[this->fileBufferPointer], bytes, sizeof(char) * size);
	this->fileBufferPointer += size;
}

void carton::Carton::readFromFileBuffer(char* output, uint64_t amount) {
	for(uint64_t i = 0; i < amount; i++) {
		output[i] = this->fileBuffer[this->fileBufferPointer++];
	}
}

void carton::Carton::readFromFileIntoFileBuffer(uint64_t amount) {
	this->initFileBuffer();
	while(this->fileBufferSize < amount) {
		this->fileBufferSize = this->fileBufferSize * 2; // new size
		this->fileBuffer = (char*)realloc(this->fileBuffer, sizeof(char) * this->fileBufferSize);
	}

	this->file.read(this->fileBuffer, amount);
	this->fileBufferSize = amount;
}

bool carton::Carton::canRead(uint64_t start, unsigned int size) {
	if(this->fileBufferSize != 0) {
		return this->fileBufferPointer < size;
	}
	else {
		return (uint64_t)this->file.tellg() < (uint64_t)start + size;
	}
}

void carton::Carton::readInflatedIntoFileBuffer(EggCompressionTypes level, unsigned int blockSize) {
	this->initFileBuffer();
	
	const uint64_t outBufferSize = 1 << 10; // write a megabyte at a time
	unsigned char outBuffer[outBufferSize];

	const uint64_t inBufferSize = 1 << 10; // read a megabyte at a time
	char inBuffer[inBufferSize];
	
	z_stream stream {
		next_in: (unsigned char*)inBuffer,
		avail_in: inBufferSize,
		next_out: outBuffer,
		avail_out: outBufferSize,
		zalloc: 0,
		zfree: 0,
	};

	inflateInit(&stream);
	uint64_t readBytes = 0;
	while(readBytes != blockSize) {
		this->file.read(inBuffer, std::min(blockSize - readBytes, inBufferSize));
		uint64_t read = this->file.gcount();
		readBytes += read;

		stream.next_in = (unsigned char*)inBuffer;
		stream.avail_in = read;

		if(read == 0) {
			break;
		}

		// write the result to the file
		int inflateRead = 0;
		do {
			inflate(&stream, 0);
			this->writeBytesToFileBuffer((char*)outBuffer, outBufferSize - stream.avail_out);
			inflateRead = outBufferSize - stream.avail_out;

			stream.next_out = outBuffer;
			stream.avail_out = outBufferSize;
		}
		while(inflateRead > 0);
	}

	int result;
	do {
		result = inflate(&stream, Z_FINISH);
		this->writeBytesToFileBuffer((char*)outBuffer, outBufferSize - stream.avail_out);

		stream.next_out = outBuffer;
		stream.avail_out = outBufferSize;
	}
	while(result == Z_OK);
	
	inflateEnd(&stream);
}
