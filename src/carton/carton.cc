#include "carton.h"

#include "file.h"

carton::Carton::Carton() {
	
}

void carton::Carton::write(string fileName) {
	this->file.open(fileName, ios_base::trunc);

	const char* magic = "CARTON";
	this->file.write(magic, 6);

	this->stringTable.write();

	// write files
	for(File* file: this->files) {
		file->write();
	}
	
	this->file.close();
}

void carton::Carton::addFile(File* file) {
	this->files.push_back(file);
}

void carton::Carton::writeBytesLittleEndian(void* data, size_t size) {
	for(int i = size - 1; i >= 0; i--) {
		this->file.write(((const char*)data + i), 1);
	}
}

// write block header
void carton::Carton::writeEgg(Egg egg) {
	this->writeData(egg.type);
	this->writeData(egg.continuedBlock);
	this->writeData(egg.compressionType);
}

void carton::Carton::writeData(unsigned char data) {
	this->writeBytesLittleEndian(&data, sizeof(unsigned char));
}

void carton::Carton::writeData(unsigned short int data) {
	this->writeBytesLittleEndian(&data, sizeof(unsigned short int));
}

void carton::Carton::writeData(unsigned long data) {
	this->writeBytesLittleEndian(&data, sizeof(unsigned long));
}

void carton::Carton::writeData(const char* data, unsigned char size) {
	this->writeData(size);
	this->file.write(data, size);
}

void carton::Carton::writeData(const char* data, unsigned short size) {
	this->writeData(size);
	this->file.write(data, size);
}

void carton::Carton::concatData(ifstream &stream) {
	this->file << stream.rdbuf();
}

void carton::Carton::concatData(zstr::ostream &stream) {
	this->file << stream.rdbuf();
}
