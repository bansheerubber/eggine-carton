#include "main.h"

#include <filesystem>
#include <tsl/robin_set.h>

#include "args.h"
#include "carton/carton.h"
#include "carton/file.h"
#include "carton/metadata/queryList.h"

void packFile(carton::Carton &carton, string path){ 
	carton::File* file = new carton::File(&carton);
	file->setFileName(path);
	carton.addFile(file);
}

void packDirectory(carton::Carton &carton, string directory) {
	carton.setPackingDirectory(directory);
	for(auto &directoryEntry: filesystem::recursive_directory_iterator(directory)) {
		if(filesystem::is_regular_file(directoryEntry.path()) && directoryEntry.path().extension() != ".metadata") {
			packFile(carton, directoryEntry.path());
		}
	}
	carton.setPackingDirectory("");
}

int main(int argc, char* argv[]) {
	vector<Argument> arguments = createArguments();
	ParsedArguments args = parseArguments(arguments, argc, argv);;
	
	if(argc < 2 || args.arguments["help"] != "") {
		help:
		printHelp(arguments, "");
		return 1;
	}

	if(string(argv[1]) == "pack") {
		if(args.files.size() < 2) {
			printf("Please specify files or directories\n");
			goto help;
		}

		carton::Carton carton;
		
		for(uint64_t i = 1; i < args.files.size(); i++) {
			string &file = args.files[i];
			if(!filesystem::exists(file)) {
				printf("File or directory %s doesn't exist\n", file.c_str());
			}
			else if(filesystem::is_directory(file)) {
				packDirectory(carton, file);
			}
			else if(filesystem::is_regular_file(file)) {
				packFile(carton, file);
			}
			else {
				printf("Could not understand file %s\n", file.c_str());
			}
		}

		carton.write(args.arguments["output"] != "" ? args.arguments["output"] : "out.carton");
	}
	else if(string(argv[1]) == "unpack") {
		if(args.files.size() < 2) {
			printf("Please specify a file to unpack\n");
			goto help;
		}

		carton::Carton carton;
		carton.read(args.files[1]);
		carton.exportFiles();
	}
	else {
		goto help;
	}
	
	return 0;
}
