#include "stringTable.h"

#include "carton.h"

carton::StringTable::StringTable(carton::Carton* carton) {
	this->carton = carton;
}

string_table_index carton::StringTable::addString(string value) {
	auto it = this->table.find(value);
	if(it == this->table.end()) { // insert
		this->table[value] = this->currentIndex;
		this->reverseTable[this->currentIndex] = value;
		return this->currentIndex++;
	}
	else {
		return it.value();
	}
}

string_table_index carton::StringTable::lookup(string value) { // alias for adding a string
	return this->addString(value);
}

void carton::StringTable::write() {
	this->carton->writeEgg(Egg {
		type: STRING_TABLE,
		continuedBlock: 0,
		compressionType: 0,
	});

	for(auto &[key, value]: this->table) {
		this->carton->writeData(value);
		string_table_string_length size = (string_table_string_length)min(key.length(), (size_t)STRING_TABLE_MAX_STRING_LENGTH); // only allow strings of size 0-255 in key string table
		this->carton->writeData(key.c_str(), size); // write string
	}
}
