#include "trim.h"

string ltrim(string s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
	}));
	return s;
}

string rtrim(string s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
	}).base(), s.end());
	return s;
}

string trim(string s) {
	s = ltrim(s);
	s = rtrim(s);
	return s;
}
