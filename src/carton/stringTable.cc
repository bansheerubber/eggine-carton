#include "stringTable.h"

#include "carton.h"

string_table_index carton::StringTable::addString(string value) {
	return this->addString(value, this->currentIndex++);
}

string_table_index carton::StringTable::addString(string value, string_table_index index) {
	auto it = this->table.find(value);
	if(it == this->table.end()) { // insert
		this->table[value] = index;
		this->reverseTable[index] = value;
		return index;
	}
	else {
		return it.value();
	}
}

string_table_index carton::StringTable::lookup(string value) { // alias for adding a string
	return this->addString(value);
}

string carton::StringTable::lookup(string_table_index index) { // alias for adding a string
	return this->reverseTable[index];
}

void carton::StringTable::write() {
	streampos eggPosition = this->carton->writeEgg(Egg {
		type: STRING_TABLE,
		blockSize: 0,
		continuedBlock: 0,
		compressionType: 0,
	});

	unsigned int totalSize = 0;
	for(auto &[key, value]: this->table) {
		totalSize += this->carton->writeNumber(value);
		string_table_string_length size = (string_table_string_length)min(key.length(), (size_t)STRING_TABLE_MAX_STRING_LENGTH); // only allow strings of size 0-255 in key string table
		totalSize += this->carton->writeString(key.c_str(), size); // write string
	}

	this->carton->writeEggSize(totalSize, eggPosition);
}

void carton::StringTable::read(Egg &header) {
	streampos start = this->carton->file.tellg();
	while(this->carton->file.tellg() < start + header.blockSize) {
		string_table_index index = this->carton->readNumber<string_table_index>();
		this->addString(this->carton->readString<unsigned char>(), index);
	}
}
