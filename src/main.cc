#include "main.h"

#include <filesystem>
#include <tsl/robin_set.h>

#include "args.h"
#include "carton/carton.h"
#include "carton/file.h"
#include "carton/metadata/queryList.h"

void frog(void* owner, carton::File* file, const char* buffer, size_t bufferSize) {
	printf("handled file %s\n", file->getFileName().c_str());
}

void packFile(carton::Carton &carton, string path){ 
	carton::File* file = new carton::File(&carton);
	file->setFileName(path);
	carton.addFile(file);
}

void packDirectory(carton::Carton &carton, string directory) {
	for(auto &directoryEntry: filesystem::recursive_directory_iterator(directory)) {
		if(filesystem::is_regular_file(directoryEntry.path()) && directoryEntry.path().extension() != ".metadata") {
			packFile(carton, directoryEntry.path());
		}
	}
}

int main(int argc, char* argv[]) {
	vector<Argument> args = createArguments();
	
	if(argc < 2) {
		help:
		printHelp(args, "");
		return 1;
	}

	if(string(argv[1]) == "pack") {
		if(argc < 3) {
			printf("Please specify files or directories\n");
			goto help;
		}

		carton::Carton carton;
		
		for(size_t i = 2; i < argc; i++) {
			if(!filesystem::exists(argv[i])) {
				printf("File or directory %s doesn't exist\n", argv[i]);
			}
			else if(filesystem::is_directory(argv[i])) {
				packDirectory(carton, string(argv[i]));
			}
			else if(filesystem::is_regular_file(argv[i])) {
				packFile(carton, string(argv[i]));
			}
			else {
				printf("Could not understand file %s\n", argv[i]);
			}
		}

		carton.write("out.carton");
	}
	else if(string(argv[1]) == "unpack") {
		if(argc < 3) {
			printf("Please specify a file to unpack\n");
			goto help;
		}

		carton::Carton carton;
		carton.read(string(argv[2]));
		carton.exportFiles();
	}
	else {
		goto help;
	}
	
	return 0;
}
