#pragma once

#include <fstream>
#include <tsl/robin_map.h>
#include <string>

using namespace std;

typedef unsigned short int string_table_index;
typedef unsigned char string_table_string_length;
#define STRING_TABLE_MAX_STRING_LENGTH 255

namespace carton {
	class StringTable {
		friend class Carton;
		
		public:
			StringTable(class Carton* carton);
			
			string_table_index addString(string value);
			string_table_index lookup(string value);
		
		private:
			class Carton* carton = nullptr;
			string_table_index currentIndex = 0;
			tsl::robin_map<string, string_table_index> table;
			tsl::robin_map<string_table_index, string> reverseTable;

			void write();
	};
};
