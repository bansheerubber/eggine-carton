#include "main.h"

#include <tsl/robin_set.h>

#include "carton/carton.h"
#include "carton/file.h"
#include "carton/metadata/queryList.h"

int main(int argc, char* argv[]) {
	carton::Carton carton;

	// for(int i = 0; i < 100000; i++) {
	// 	carton::File* file = new carton::File(&carton);
	// 	file->setFileName("spritesheet.png");

	// 	carton.addFile(file);
	// }

	// carton.write("test.carton");

	carton.read("test.carton");

	DynamicArray<carton::Metadata*, void> test = carton.database.get()->has("fileName")->exec();
	// for(size_t i = 0; i < test.head; i++) {
	// 	printf("%s\n", test[i]->getMetadata("fileName").c_str());
	// }
	
	return 0;
}
