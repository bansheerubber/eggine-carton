#pragma once

#include <string>

#include "metadata.h"

using namespace std;

namespace carton {
	class File {
		friend class Carton;
		
		public:
			File(class Carton* carton);
			
			Metadata metadata;
			void setFileName(string fileName);
			string getFileName();
		
		private:
			string fileName;
			class Carton* carton = nullptr;

			void write();
	};
};
