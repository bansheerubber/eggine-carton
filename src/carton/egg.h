#pragma once

#include <cstdint>

namespace carton {
	enum EggTypes {
		INVALID_EGG,
		STRING_TABLE,
		METADATA,
		FILE,
		FILE_LIST,
	};

	enum EggCompressionTypes {
		NO_COMPRESSION,
		ZLIB_LEVEL_0,
		ZLIB_LEVEL_1,
		ZLIB_LEVEL_2,
		ZLIB_LEVEL_3,
		ZLIB_LEVEL_4,
		ZLIB_LEVEL_5,
		ZLIB_LEVEL_6,
		ZLIB_LEVEL_7,
		ZLIB_LEVEL_8,
		ZLIB_LEVEL_9,
	};
	
	struct Egg { // an egg is a block of data
		unsigned short int type;
		unsigned int blockSize;

		// if this block is separated into different blocks, then this specifies where its continued
		// bit 0 is used to determine if this block is a continued one or not
		uint64_t continuedBlock;
		unsigned short int compressionType;

		int operator==(const Egg &other) {
			return this->type == other.type && this->continuedBlock == other.continuedBlock && this->compressionType == other.compressionType;
		}
	};

	class EggContents {
		friend class Carton;

		public:
			EggContents();
			EggContents(class Carton* carton);
			virtual ~EggContents() = 0;

		protected:
			class Carton* carton = nullptr;
			
			virtual void write() = 0;
			virtual void read(Egg &header, unsigned int size) = 0;
	};
};
