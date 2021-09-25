#include "main.h"

#include <tsl/robin_set.h>

#include "carton/carton.h"
#include "carton/file.h"
#include "carton/metadata/queryList.h"

int main(int argc, char* argv[]) {
	carton::Carton carton;

	// carton::File* file = new carton::File(&carton);
	// file->setFileName("spritesheet.png");

	// carton.addFile(file);
	// carton.write("test.carton");

	carton.read("test.carton");

	tsl::robin_set<carton::Metadata*> test = carton.database.get()->has("fileName")->exec();
	for(carton::Metadata* metadata: test) {
		printf("%s\n", metadata->getMetadata("fileName").c_str());
	}
	
	return 0;
}
