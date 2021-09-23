#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "egg.h"
#include "stringTable.h"

using namespace std;

namespace carton {
	class Carton {
		friend class File;
		friend class Metadata;
		friend StringTable;
		
		public:
			Carton();

			vector<EggContents*> contents;

			void write(string fileName);
			void read(string fileName);
			void addFile(class File* file);
		
		private:
			fstream file;
			StringTable stringTable = StringTable(this);
			vector<class File*> files;
			
			// write methods
			streampos writeEgg(Egg egg);
			void writeEggSize(unsigned int size, streampos eggPosition);

			template<class T>
			unsigned int writeNumber(T number) {
				for(int i = sizeof(T) - 1; i >= 0; i--) {
					this->file.write(((const char*)&number + i), 1);
				}
				return sizeof(T);
			}

			template<class T>
			unsigned int writeString(const char* data, T size) {
				this->writeNumber<T>(size);
				this->file.write(data, size);
				return sizeof(T) + size;
			}

			// read methods
			Egg readEgg();

			template<class T>
			T readNumber() {
				char bytes[sizeof(T)];
				this->file.read(bytes, sizeof(T));
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
				this->file.read(output, size);
				return string(output, size);
			}
	};
};
