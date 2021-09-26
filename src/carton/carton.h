#pragma once

#include <fstream>
#include <string>
#include <tsl/robin_map.h>
#include <tsl/robin_set.h>
#include <vector>

#include "egg.h"
#include "fileList.h"
#include "metadata/metadataDatabase.h"
#include "stringTable.h"

using namespace std;

namespace carton {
	typedef void (*file_extension_handler)(class File* file, const char* buffer, size_t fileBufferSize);
	
	class Carton {
		friend class File;
		friend class FileList;
		friend class Metadata;
		friend StringTable;
		friend size_t __writeDeflated(carton::Carton* carton, istream* input, const char* buffer, size_t bufferSize, carton::EggCompressionTypes level);
		
		public:
			Carton();

			tsl::robin_set<EggContents*> contents;
			tsl::robin_map<unsigned long, EggContents*> positionToContents;
			tsl::robin_map<EggContents*, unsigned long> contentsToEnd;
			tsl::robin_map<unsigned long, EggContents*> endToContents;

			void write(string fileName);
			void read(string fileName);
			void addFile(class File* file);
			class File* readFile(string fileName);
			void addExtensionHandler(string extension, file_extension_handler handler);

			MetadataDatabase database = MetadataDatabase(this);
		
		private:
			const unsigned int version = 1;
			
			fstream file;
			StringTable stringTable = StringTable(this);
			FileList fileList = FileList(this);
			unsigned long fileListPointerPosition = 0;

			vector<class File*> files;

			tsl::robin_map<string, file_extension_handler> extensionHandlers;

			char* fileBuffer = nullptr; // home for temp data, we can write/read from it using the write/read commands
			size_t fileBufferSize = 0;
			size_t fileBufferPointer = 0;

			void initFileBuffer();
			void deleteFileBuffer();
			void commitFileBuffer();
			size_t commitDeflatedFileBuffer(EggCompressionTypes compression);
			void writeToFileBuffer(char byte);
			void writeBytesToFileBuffer(char* bytes, size_t size);
			void readFromFileBuffer(char* output, size_t amount);
			void readFromFileIntoFileBuffer(size_t amount);
			
			// write methods
			streampos writeEgg(Egg egg);
			void writeEggSize(unsigned int size, streampos eggPosition);

			template<class T>
			unsigned int writeNumber(T number) {
				for(int i = sizeof(T) - 1; i >= 0; i--) {
					if(this->fileBufferSize != 0) {
						char byte = ((const char*)&number + i)[0]; // shout-out to reverse engineering for teaching me the absolutely correct way of doing this
						this->writeToFileBuffer(byte);
					}
					else {
						this->file.write(((const char*)&number + i), 1);
					}
				}
				return sizeof(T);
			}

			template<class T>
			unsigned int writeString(const char* data, T size) {
				this->writeNumber<T>(size);

				if(this->fileBufferSize != 0) {
					for(T i = 0; i < size; i++) {
						this->writeToFileBuffer(data[i]);
					}
				}
				else {
					this->file.write(data, size);
				}
				
				return sizeof(T) + size;
			}

			size_t writeDeflated(istream &stream, EggCompressionTypes level);
			size_t writeDeflated(char* buffer, size_t size, EggCompressionTypes level);

			// read methods
			Egg readEgg();
			EggContents* parseEggContents();

			template<class T>
			T readNumber() {
				char bytes[sizeof(T)];

				if(this->fileBufferSize != 0) {
					this->readFromFileBuffer(bytes, sizeof(T));
				}
				else {
					this->file.read(bytes, sizeof(T));
				}
				
				char reversed[sizeof(T)];
				for(int i = sizeof(T) - 1, j = 0; i >= 0; i--, j++) {
					reversed[j] = bytes[i];
				}

				return *((T*)(reversed));
			}

			template<class T>
			string readString() {
				// read the size of the string
				T size = this->readNumber<T>();
				char output[size];

				if(this->fileBufferSize != 0) {
					this->readFromFileBuffer(output, size);
				}
				else {
					this->file.read(output, size);
				}

				return string(output, size);
			}

			bool canRead(streampos start, unsigned int size);

			void readDeflatedIntoFileBuffer(EggCompressionTypes level, unsigned int blockSize);
	};
};
