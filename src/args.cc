#include "args.h"

std::vector<Argument> createArguments() {
	std::vector<Argument> output;

	output.push_back((Argument){
		name: "help",
		shortcut: "h",
		help: "Show help message"
	});

	output.push_back((Argument){
		name: "output",
		shortcut: "o",
		help: "Output file for pack",
		needsInput: true,
	});

	return output;
}

Argument getArgumentFromName(std::vector<Argument> &arguments, std::string &name) {
	// not the fastest code but doesn't matter, we're dealing with like 10 arguments max
	for(Argument argument: arguments) {
		if(argument.name == name) {
			return argument;
		}
	}
	return {};
}

void printHelp(std::vector<Argument> &arguments, std::string argumentName) {
	std::cout << "usage: carton [pack | unpack] [files or directories]" << std::endl;

	int helpPosition = 50;
	for(Argument argument: arguments) {
		std::string parameters = "--" + argument.name;
		if(argument.helpVariable != "") {
			parameters += " [" + argument.helpVariable + "]";
		}

		// handle shortcut
		if(argument.shortcut != "") {
			parameters = "-" + argument.shortcut;

			if(argument.helpVariable != "") {
				parameters += " [" + argument.helpVariable + "]";
			}

			parameters += ", --" + argument.name;

			if(argument.helpVariable != "") {
				parameters += " [" + argument.helpVariable + "]";
			}
		}
		
		std::cout << "    ";
		std::cout << parameters;
		for(uint64_t i = 0; i < helpPosition - parameters.length(); i++) {
			std::cout << " ";
		}
		std::cout << argument.help << std::endl;
	}
}

ParsedArguments parseArguments(std::vector<Argument> &arguments, int argc, char* argv[]) {
	ParsedArguments output = {
		argumentError: false,
	};

	Argument foundArgument;
	for(int i = 1; i < argc; i++) {
		std::string cliArgument(argv[i]);

		// search if the string argument matches any of the struct arguments
		if(cliArgument[0] == '-') {
			// push argument
			if(foundArgument.name != "") {
				if(foundArgument.needsInput) { // fail if we got no input when we need it
					output = (ParsedArguments){
						argumentError: true,
					};
					return output;
				}
				
				output.arguments.insert(std::pair<std::string, std::string>(foundArgument.name, "true"));
			}
			
			foundArgument = (Argument){};
			for(Argument argument: arguments) {
				if("--" + argument.name == cliArgument || (argument.shortcut != "" && "-" + argument.shortcut == cliArgument)) {
					foundArgument = argument;
				}
			}

			// fail if we got an unknown argument
			if(foundArgument.name == "") {
				output = (ParsedArguments){
					argumentError: true,
				};
				return output;
			}
		}
		// if we have a random string that isn't associated with an argument, treat it as an input file
		else if(foundArgument.name == "" || !foundArgument.needsInput) {
			output.files.push_back(cliArgument);
		}
		else {
			output.arguments.insert(std::pair<std::string, std::string>(foundArgument.name, cliArgument));
			foundArgument = (Argument){};
		}
	}

	// push argument
	if(foundArgument.name != "") {
		if(foundArgument.needsInput) { // fail if we got no input when we need it
			output = (ParsedArguments){
				argumentError: true,
			};
			return output;
		}
		
		output.arguments.insert(std::pair<std::string, std::string>(foundArgument.name, "true"));
	}

	return output;
}