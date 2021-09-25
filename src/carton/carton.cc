#include "carton.h"

#include <cstring>
#include <fstream>
#include <zlib.h>

#include "file.h"

carton::Carton::Carton() {
	
}

void carton::Carton::write(string fileName) {
	this->file.open(fileName, ios_base::out);

	const char* magic = "CARTON";
	this->file.write(magic, 6);

	this->writeNumber(this->version);

	this->fileListPointerPosition = this->file.tellp();
	unsigned long fileListPointer = 0;
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

	unsigned int version = this->readNumber<unsigned int>();
	if(this->version != version) {
		printf("version mismatch: %u != %u\n", this->version, version);
		exit(1);
	}

	unsigned long fileListPointer = this->readNumber<unsigned long>();

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
		Metadata* metadata = (Metadata*)this->parseEggContents();
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

carton::EggContents* carton::Carton::parseEggContents() {
	Egg egg = this->readEgg();
	EggContents* output = nullptr;

	unsigned int size = egg.blockSize;
	if(egg.compressionType != NO_COMPRESSION) {
		this->readDeflatedIntoFileBuffer((EggCompressionTypes)egg.compressionType, egg.blockSize);
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
			output = new File(this);
			output->read(egg, size);
			break;
		}

		case FILE_LIST: {
			this->fileList.read(egg, size);
			output = &this->fileList;
			break;
		}

		default: {
			printf("unexpected egg type '%d'\n", egg.type);
			exit(1);
		}
	}

	if(egg.compressionType != NO_COMPRESSION) {
		this->deleteFileBuffer(); // clean up the mess
	}

	return output;
}

size_t carton::__writeDeflated(carton::Carton* carton, istream* input, const char* buffer, size_t bufferSize, carton::EggCompressionTypes level) {
	if(level < carton::ZLIB_LEVEL_0 || level > carton::ZLIB_LEVEL_9) {
		printf("invalid deflate level %d\n", level);
		exit(1);
	}
	
	const size_t outBufferSize = 1 << 20; // write a megabyte at a time
	unsigned char outBuffer[outBufferSize];

	const size_t inBufferSize = 1 << 20; // read a megabyte at a time
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

	streampos start = carton->file.tellp();
	deflateInit(&stream, level - carton::ZLIB_LEVEL_0);

	size_t totalRead = 0;
	while(true) {
		if(input != nullptr) {
			input->read(inBuffer, inBufferSize);
			streamsize readCount = input->gcount();
			
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
			stream.avail_in = min(bufferSize - totalRead, inBufferSize); // try to read a megabyte at a time

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

	return carton->file.tellp() - start;
}

size_t carton::Carton::writeDeflated(istream &input, EggCompressionTypes level) {
	return __writeDeflated(this, &input, nullptr, 0, level);
}

size_t carton::Carton::writeDeflated(char* buffer, size_t size, EggCompressionTypes level) {
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

size_t carton::Carton::commitDeflatedFileBuffer(EggCompressionTypes compression) {
	size_t size = this->writeDeflated(this->fileBuffer, this->fileBufferPointer, compression);
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

void carton::Carton::writeBytesToFileBuffer(char* bytes, size_t size) {
	size_t newSize = this->fileBufferPointer + size;
	while(this->fileBufferSize < newSize) {
		this->fileBufferSize = this->fileBufferSize * 2; // new size
		this->fileBuffer = (char*)realloc(this->fileBuffer, sizeof(char) * this->fileBufferSize);
	}

	memcpy(&this->fileBuffer[this->fileBufferPointer], bytes, sizeof(char) * size);
	this->fileBufferPointer += size;
}

void carton::Carton::readFromFileBuffer(char* output, size_t amount) {
	for(size_t i = 0; i < amount; i++) {
		output[i] = this->fileBuffer[this->fileBufferPointer++];
	}
}

void carton::Carton::readFromFileIntoFileBuffer(size_t amount) {
	this->initFileBuffer();
	while(this->fileBufferSize < amount) {
		this->fileBufferSize = this->fileBufferSize * 2; // new size
		this->fileBuffer = (char*)realloc(this->fileBuffer, sizeof(char) * this->fileBufferSize);
	}

	this->file.read(this->fileBuffer, amount);
	this->fileBufferSize = amount;
}

bool carton::Carton::canRead(streampos start, unsigned int size) {
	if(this->fileBufferSize != 0) {
		return this->fileBufferPointer < size;
	}
	else {
		return this->file.tellg() < start + size;
	}
}

void carton::Carton::readDeflatedIntoFileBuffer(EggCompressionTypes level, unsigned int blockSize) {
	this->initFileBuffer();
	
	const size_t outBufferSize = 1 << 20; // write a megabyte at a time
	unsigned char outBuffer[outBufferSize];

	const size_t inBufferSize = 1 << 20; // read a megabyte at a time
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
	streampos start = this->file.tellg();
	size_t readBytes = 0;
	while(readBytes != blockSize) {
		this->file.read(inBuffer, min(blockSize - readBytes, inBufferSize));
		size_t read = this->file.gcount();
		readBytes += read;

		stream.next_in = (unsigned char*)inBuffer;
		stream.avail_in = read;

		if(read == 0) {
			break;
		}

		// write the result to the file
		do {
			int result = inflate(&stream, 0);
			this->writeBytesToFileBuffer((char*)outBuffer, outBufferSize - stream.avail_out);

			stream.next_out = outBuffer;
			stream.avail_out = outBufferSize;
		}
		while(stream.avail_in > 0);
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
