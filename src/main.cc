#include "main.h"

#include "carton/carton.h"
#include "carton/file.h"

int main(int argc, char* argv[]) {
	carton::Carton carton;

	carton::File* file = new carton::File(&carton);
	file->setFileName("spritesheet.png");

	carton.addFile(file);
	carton.write("test.carton");

	// carton.read("test.carton");
	
	return 0;
}
