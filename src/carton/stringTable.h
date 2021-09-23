#pragma once

#include <fstream>
#include <tsl/robin_map.h>
#include <string>

#include "egg.h"

using namespace std;

typedef unsigned short int string_table_index;
typedef unsigned char string_table_string_length;
#define STRING_TABLE_MAX_STRING_LENGTH 255

namespace carton {
	class StringTable: public EggContents {
		friend class Carton;
		
		public:
			using EggContents::EggContents;
			
			string_table_index addString(string value);
			string_table_index lookup(string value);
			string lookup(string_table_index);
		
		private:
			string_table_index currentIndex = 0;
			tsl::robin_map<string, string_table_index> table;
			tsl::robin_map<string_table_index, string> reverseTable;

			string_table_index addString(string value, string_table_index index);
			void write();
			void read(Egg &header, unsigned int size);
	};
};
