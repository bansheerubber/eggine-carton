#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>

struct Argument {
	std::string name;
	std::string shortcut;
	std::string helpVariable;
	std::string help;
	bool needsInput;
	bool hide;
};

struct ParsedArguments {
	std::map<std::string, std::string> arguments;
	std::vector<std::string> files;
	bool argumentError;
};

std::vector<Argument> createArguments();
Argument getArgumentFromName(std::vector<Argument> &arguments, std::string &name);
void printHelp(std::vector<Argument> &arguments, std::string argumentName = "");
ParsedArguments parseArguments(std::vector<Argument> &arguments, int argc, char* argv[]);