#include "main.h"

#include <tsl/robin_set.h>

#include "carton/carton.h"
#include "carton/file.h"
#include "carton/metadata/queryList.h"

void frog(carton::File* file, const char* buffer, size_t bufferSize) {
	printf("handled file %s\n", file->getFileName().c_str());
}

int main(int argc, char* argv[]) {
	carton::Carton carton;
	carton.addExtensionHandler(".png", &frog);

	// carton::File* file = new carton::File(&carton);
	// file->setFileName("spritesheet.png");
	// carton.addFile(file);
	// carton.write("test.carton");

	carton.read("test.carton");

	DynamicArray<carton::Metadata*, void> test = carton.database.get()->has("fileName")->exec();
	for(size_t i = 0; i < test.head; i++) {
		carton.readFile(test[i]->getMetadata("fileName"));
	}
	
	return 0;
}
