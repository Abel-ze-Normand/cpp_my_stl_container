// my_stl_container.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

//
//  main.cpp
//  TRPO05
//
//  Created by Abel Normand on 14.04.16.
//  Copyright © 2016 Abel Normand. All rights reserved.
//

#include <iostream>
#include <algorithm>
#include <assert.h>

template<typename T>
class block_vector {
public:
	struct OverloadBlockException { };
	struct OutOfBoundsException { };

	struct block {
		T* array = 0;
		int count = 0;
		block *next, *prev;
		const int MAXCOUNT = 10;
		block() {
			array = (T*)malloc(MAXCOUNT * sizeof(T));
		}
		T& operator[](int i) {
			return *array[i];
		}
		void push_back(T& val) {
			if (!count == MAXCOUNT) {
				array[count++] = val;
			}
			else
				throw new OverloadBlockException();
		}
		~block() {
			free(array);
		}
	};


	typedef T value_type;
	typedef T& reference;
	typedef const T& const_reference;
	typedef size_t size_type;

	class iterator {
	public:
		typedef iterator self_type;
		typedef int difference_type;
		typedef T value_type;
		typedef T& reference;
		typedef T* pointer;
		typedef std::random_access_iterator_tag iterator_category;

		iterator() : ptr(0), current_position(0), current_block(0) {};
		iterator(pointer _ptr, int position, block _bl) : ptr(_ptr), current_position(position), current_block(&_bl) {};
		iterator(const iterator & i) : ptr(i.ptr), current_position(i.current_position), current_block(i.current_block) { };
		~iterator() { };

		self_type operator++() {
			self_type i = *this;
			ptr = move_next();
			return i;
		}
		self_type operator++(int stub) {
			ptr = move_next();
			return *this;
		}
		self_type operator--() {
			self_type i = *this;
			ptr = move_back();
			return i;
		}
		self_type operator--(int stub) {
			ptr = move_back();
			return *this;
		}
		self_type operator+(const int b) const {
			assert(current_block->count > 0)
			int i = b, c = current_position;
			self_type mover = this;
			while (true) {
				if (c + i > mover.current_block->count) {
					c = 0;
					i -= (mover.current_block->count - c);
					if (mover.current_block->next)
						mover.current_block = mover.current_block->next;
					else
						throw new OutOfBoundsException();
				}
				else
					return self_type(mover.current_block->array + c + i, c + i, mover.current_block);
			}
		}
		self_type operator-(const int b) const {
			assert(current_block->count > 0)
			int i = b, c = current_position;
			self_type mover = this;
			while (true) {
				if (c - i < 0) {
					if (mover.current_block->prev)
						mover.current_block = mover.current_block->prev;
					else
						throw new OutOfBoundsException();
					i -= c;
					c = mover.current_block->count;
				}
				else
					return self_type(mover.current_block->array + c - i, c - i, mover.current_block);
			}
		}
		difference_type operator-(const self_type& b) const {
			int accumulator = 0;
			int c = current_position;
			self_type mover = this;
			while (true) {
				if (mover.current_block != b.current_block) {
					if (mover.current_block->prev)
						mover.current_block = mover.current_block->prev;
					else
						throw new OutOfBoundsException();
					accumulator += c;
					c = mover.current_block->count;
				}
				else
					return accumulator + c - b.current_position;
			}
		}
		reference operator*() {
			return *ptr;
		}
		pointer operator->() {
			return ptr;
		}
		bool operator==(self_type& a) {
			return ptr == a.ptr;
		}
		bool operator!=(self_type& a) {
			return ptr != a.ptr;
		}
		// --------this--------a--------
		// if we move pointer backwards and dont impact a.block and encounter 0 => (this) is less
		bool operator<(self_type& a) { 
			self_type mover = this;
			if (mover.current_block == a.current_block)
				return mover.current_position < a.current_position;
			while (true) {
				if (mover.current_block == a.current_block)
					return false;
				if (mover.current_block->prev)
					mover.current_block = mover.current_block->prev;
				else
					return true;
			}
		}
		bool operator>(self_type& a) {
			self_type mover = this;
			if (mover.current_block == a.current_block)
				return mover.current_position > a.current_position;
			while (true) {
				if (mover.current_block == a.current_block)
					return false;
				if (mover.current_block->next)
					mover.current_block = mover.current_block->next;
				else
					return true;
			}
		}
		bool operator<=(self_type& a) {
			return (this == a || this < a);
		}
		bool operator>=(self_type& a) {
			return (this == a || this > a);
		}
	private:
		pointer ptr;
		int current_position;
		block* current_block;
		pointer move_next() throw(OutOfBoundsException) {
			if (current_position < current_block->count) {
				return &current_block[current_position++];
			}
			else if (current_block->next != 0) {
				current_block = current_block->next;
				current_position = 0;
				return &current_block[current_position++];
			}
			else {
				throw new OutOfBoundsException();
			}
		}
		pointer move_back() throw(OutOfBoundsException) {
			if (current_position >= 0) {
				return &current_block[current_position--];
			}
			else if (current_block->prev != 0) {
				current_block = current_block->prev;
				current_position = current_block->count;
				return &current_block[--current_position];
			}
			else {
				throw new OutOfBoundsException();
			}
		}
	}; // end of iterator class 

	block_vector() : length(0), blocks_count(0), blocks(0) { };

private:
	block* blocks = 0;
	int blocks_count = 0;
	int length = 0;
}; // end of block_vector class



int main(int argc, const char * argv[]) {
	block_vector<int> bv;
}

