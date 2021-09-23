#include "file.h"

#include <fstream>
#include <zlib.h>

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
		streampos eggPosition = this->carton->writeEgg(Egg {
			type: FILE,
			blockSize: 0,
			continuedBlock: 0,
			compressionType: ZLIB_BEST,
		});

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

		streampos start = this->carton->file.tellp();
		ifstream file(this->fileName);
		deflateInit(&stream, Z_BEST_COMPRESSION);

		while(true) {
			file.read(inBuffer, inBufferSize);
			streamsize readCount = file.gcount();
			
			stream.next_in = (unsigned char*)inBuffer;
			stream.avail_in = readCount;

			if(readCount == 0) {
				break;
			}

			// write the result to the file
			do {
				deflate(&stream, 0);
				this->carton->file.write((char*)outBuffer, outBufferSize - stream.avail_out);

				stream.next_out = outBuffer;
				stream.avail_out = outBufferSize;
			}
			while(stream.avail_in > 0);
		}

		int result;
		do {
			result = deflate(&stream, Z_FINISH);
			this->carton->file.write((char*)outBuffer, outBufferSize - stream.avail_out);

			stream.next_out = outBuffer;
			stream.avail_out = outBufferSize;
		}
		while(result == Z_OK);
		
		deflateEnd(&stream);
		file.close();

		this->carton->writeEggSize(this->carton->file.tellp() - start, eggPosition);
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

void carton::File::read(Egg &header) {
	this->setFileName(this->metadata->getMetadata("fileName"));
	
	switch(header.compressionType) {
		case NO_COMPRESSION: {
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
			break;
		}

		case ZLIB_BEST: {
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
			ofstream file("output/" + this->getFileName());
			streampos start = this->carton->file.tellg();
			size_t readBytes = 0;
			while(readBytes != header.blockSize) {
				this->carton->file.read(inBuffer, min(header.blockSize - readBytes, inBufferSize));
				size_t read = this->carton->file.gcount();
				readBytes += read;

				stream.next_in = (unsigned char*)inBuffer;
				stream.avail_in = read;

				if(read == 0) {
					break;
				}

				// write the result to the file
				do {
					inflate(&stream, 0);
					file.write((char*)outBuffer, outBufferSize - stream.avail_out);

					stream.next_out = outBuffer;
					stream.avail_out = outBufferSize;
				}
				while(stream.avail_in > 0);

				int result;
				do {
					result = inflate(&stream, Z_FINISH);
					file.write((char*)outBuffer, outBufferSize - stream.avail_out);

					stream.next_out = outBuffer;
					stream.avail_out = outBufferSize;
				}
				while(result == Z_OK);
				
				inflateEnd(&stream);

				file.close();
			}
			
			break;
		}
	}
}
