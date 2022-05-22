#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <utility>

#define DYNAMIC_ARRAY_MAX_SIZE 5000000

template <typename T>
class DynamicArray {
	public:
		uint64_t head;
		uint64_t size;

		DynamicArray() {
			this->array = nullptr;
			this->dontDelete = true;
		}

		DynamicArray(uint64_t size) {
			this->head = 0;
			this->size = size;

			this->constructArray();
		}

		~DynamicArray() {
			if(this->dontDelete) {
				this->dontDelete = false;
				return;
			}
			
			if(this->array != nullptr) {
				free(this->array);
				this->array = nullptr;
			}
		}

		void pushed() {
			this->head++;

			if(this->head == this->size) {
				this->allocate(this->size * 2);
			}
		}

		void popped() {
			this->head--;
		}

		void allocate(uint64_t amount) {
			if(amount < this->size) {
				return;
			}
			
			if(amount > DYNAMIC_ARRAY_MAX_SIZE) {
				printf("stack overflow\n");
				exit(1);
			}
			
			T* array = (T*)realloc(this->array, sizeof(T) * amount);
			if(array == NULL) {
				printf("invalid dynamic array realloc\n");
				exit(1);
			}
			this->array = array;

			for(uint64_t i = this->size; i < amount; i++) {
				::new (static_cast<void*>(&this->array[i])) T();
			}
			this->size = amount;
		}

		void remove(T entry) {
			int64_t index = this->index(entry);
			if(index != -1) {
				this->shift(index + 1, -1);
			}
		}

		int64_t index(T entry) {
			for(uint64_t i = 0; i < this->head; i++) {
				if(entry == this->array[i]) {
					return i;
				}
			}
			return -1;
		}

		void shift(int64_t index, int64_t amount) {
			int64_t end = (int64_t)this->head;

			for(int i = 0; i < amount; i++) {
				this->pushed(); // allocate space
			}

			// start from the end for shifting right
			if(amount >= 0) {
				for(int i = end - 1; i >= index; i--) {
					this->array[i + amount] = std::move(this->array[i]); // move
				}
			}
			else {
				for(int i = index; i < end; i++) {
					this->array[i + amount] = std::move(this->array[i]); // move
				}
			}

			for(int i = index; i < index + amount; i++) {
				::new (static_cast<void*>(&this->array[i])) T; // re-initialize entries
			}

			if(amount < 0) { // pop for shift lefts
				for(int i = end - 1; i >= end + amount; i--) {
					::new (static_cast<void*>(&this->array[i])) T;
					this->popped();
				}
			}
		}

		T& operator[](uint64_t index) {
			return this->array[index];
		}
	
	private:
		void constructArray() {
			T* array = (T*)malloc(sizeof(T) * this->size);
			if(array == NULL) {
				printf("invalid dynamic array malloc\n");
				exit(1);
			}
			this->array = array;

			for(uint64_t i = 0; i < this->size; i++) {
				::new (static_cast<void*>(&this->array[i])) T();
			}
		}

		bool dontDelete = false;
		T* array;
};
