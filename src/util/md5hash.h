#pragma once

#ifndef __switch__
#include <fstream>
#include <openssl/md5.h>

inline void md5hash(std::string fileName, unsigned char* result) {
	MD5_CTX context;
	const unsigned int size = 32768;
	unsigned char* data = new unsigned char[size];
	std::ifstream file(fileName, std::ios_base::binary);

	MD5_Init(&context);
	while(!file.eof()) {
		file.read((char*)data, size);
		MD5_Update(&context, data, file.gcount());
	}

	MD5_Final(result, &context);
}
#endif
