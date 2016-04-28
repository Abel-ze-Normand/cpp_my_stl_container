//
//  main.cpp
//  TRPO05
//
//  Created by Abel Normand on 14.04.16.
//  Copyright © 2016 Abel Normand. All rights reserved.
//

#include <iostream>
#include <assert.h>
#include <iterator>

template<typename T>
class block_vector {
public:
	struct OverloadBlockException { };
	struct OutOfBoundsException { };
	struct EmptyBlockException { };

	class block {
	public:
		T* array;
		int count;
		block *next, *prev;
		static const int MAXCOUNT = 10;
		block() : array(0), count(0), next(0), prev(0) {
			array = new T[MAXCOUNT];
		}
		block(block const & b) {
			array = new T[MAXCOUNT];
			std::copy(&b.array[0], &b.array[MAXCOUNT], array);
			count = b.count;
			next = b.next;
			prev = b.prev;
		}
		block& operator=(const block & a) {
			delete[] array;
			array = new T[MAXCOUNT];
			std::copy(&a.array[0], &a.array[MAXCOUNT], array);
			count = a.count;
			prev = a.prev;
			next = a.next;
			return *this;
		}
		T& operator[](int i) {
			return array[i];
		}
		void push_back(const T& val) {
			if (count != MAXCOUNT) {
				array[count++] = val;
			}
			else {
				throw OverloadBlockException();
			}
		}
		void pop_front() {
			if (count > 1) {
				for (int i = 0; i < count - 1; i++) {
					array[i] = array[i + 1];
				}
				count--;
			}
			else {
				throw EmptyBlockException();
			}
		}
		void insert(const T & val, int position) {
			if (count == MAXCOUNT) {
				throw OverloadBlockException();
			}
			else {
				for (int i = count; i > position; i--) {
					array[i] = array[i-1];
				}
				array[position] = val;
				count++;
			}
		}
		void remove_at(int position) {
			if (count > 1) {
				delete(array[position]);
				for (int i = position; i < count; i++) {
					array[i] = array[i + 1];
				}
				count--;
			}
			else {
				throw EmptyBlockException();
			}
		}
		T& front() const {
			return array[0];
		}
		T& last() const {
			return array[count];
		}
		~block() {
			delete[] array;
		}
	private:
		void swap(block & s) {
			std::swap(array, s.array);
			std::swap(count, s.count);
			std::swap(next, s.next);
			std::swap(prev, s.prev);
		}
	};
	typedef T value_type;
	typedef T& reference;
	typedef const T& const_reference;
	typedef size_t size_type;
	typedef block_vector self_type;
	//iterators
	class iterator {
	public:
		typedef iterator self_type;
		typedef int difference_type;
		typedef T value_type;
		typedef T& reference;
		typedef T* pointer;
		typedef std::random_access_iterator_tag iterator_category;

		pointer ptr;
		int current_position;
		block* current_block;

		iterator() : ptr(0), current_position(0), current_block(0) { };
		iterator(pointer _ptr, int position, block* _bl) : ptr(_ptr), current_position(position), current_block(_bl) { };
		iterator(const iterator & i) : ptr(i.ptr),
			current_position(i.current_position), current_block(i.current_block) { };
		~iterator() { };

		self_type operator++(int stub) {
			self_type i(*this);
			ptr = move_next();
			return i;
		}
		self_type operator++() {
			ptr = move_next();
			return *this;
		}
		self_type operator--(int stub) {
			self_type i(*this);
			ptr = move_back();
			return i;
		}
		self_type operator--() {
			ptr = move_back();
			return *this;
		}
		self_type operator+(const int b) {
			assert(current_block->count > 0);
			int i = b, c = current_position;
			self_type mover(*this);
			while (true) {
				if (c + i > mover.current_block->count) {
					c = 0;
					i -= (mover.current_block->count - c);
					if (mover.current_block->next) {
						mover.current_block = mover.current_block->next;
					}
					else {
						throw new OutOfBoundsException();
					}
				}
				else {
					return self_type(mover.current_block->array + c + i, c + i, mover.current_block);
				}
			}
		}
		self_type operator-(const int b) {
			assert(current_block->count > 0);
			int i = b, c = current_position;
			self_type mover(*this);
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
		difference_type operator-(const self_type& b) {
			int accumulator = 0;
			int c = current_position;
			self_type mover(*this);
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
		bool operator==(const self_type& a) const {
			return ptr == a.ptr;
		}
		bool operator!=(const self_type& a) const {
			return ptr != a.ptr;
		}
		// --------this--------a--------
		// if we move pointer backwards and dont impact a.block and encounter 0 => (this) is less
		bool operator<(const self_type& a) const {
			self_type mover(*this);
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
		bool operator>(const self_type& a) const {
			self_type mover(*this);
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
		bool operator<=(const self_type& a) const {
			return (*this == a || *this < a);
		}
		bool operator>=(const self_type& a) const {
			return (*this == a || *this > a);
		}
		self_type operator+=(int n) {
			for (int i = 0; i < n; i++) {
				move_next();
			}
			return *this;
		}
		self_type operator-=(int n) {
			for (int i = 0; i < n; i++) {
				move_back();
			}
			return *this;
		}
		// i'm not sure how to make random access right
		reference operator[](int j) {
			//rewind to begin??
			self_type a(*this);

			for (int i = 0; i < j; i++) {
				a.move_next();
			}
			return *a;
		}
		void swap(reference a, reference b) {
			self_type c = a;
			a = b;
			b = c;
		}
	private:
		pointer move_next() {
			if (++current_position < current_block->count) {
				return ptr = &((*current_block)[current_position]);
			}
			else if (current_block->next != 0) {
				current_block = current_block->next;
				current_position = 0;
				return ptr = &((*current_block)[current_position]);
			}
			else {
				return ptr = &((*current_block)[current_position]);
			}
		}
		pointer move_back() {
			if (--current_position > 0) {
				return ptr = &((*current_block)[current_position]);
			}
			else if (current_block->prev != 0) {
				current_block = current_block->prev;
				current_position = current_block->count-1;
				return ptr = &((*current_block)[current_position]);
			}
			else {
				return ptr = &((*current_block)[current_position]);
			}
		}
	}; // end of iterator class
	class const_iterator {
	public:
		typedef const_iterator self_type;
		typedef int difference_type;
		typedef T value_type;
		typedef T& reference;
		typedef T* pointer;
		typedef std::random_access_iterator_tag iterator_category;

		pointer ptr;
		int current_position;
		block* current_block;

		const_iterator() : ptr(0), current_position(0), current_block(0) { };
		const_iterator(pointer _ptr, int position, block* _bl) : ptr(_ptr), current_position(position), current_block(_bl) { };
		const_iterator(const iterator & i) : ptr(i.ptr),
									   current_position(i.current_position), current_block(i.current_block) { };
		~iterator() { };

		self_type operator++(int stub) {
			self_type i(*this);
			ptr = move_next();
			return i;
		}
		self_type operator++() {
			ptr = move_next();
			return *this;
		}
		self_type operator--(int stub) {
			self_type i(*this);
			ptr = move_back();
			return i;
		}
		self_type operator--() {
			ptr = move_back();
			return *this;
		}
		self_type operator+(const int b) {
			assert(current_block->count > 0);
			int i = b, c = current_position;
			self_type mover(*this);
			while (true) {
				if (c + i > mover.current_block->count) {
					c = 0;
					i -= (mover.current_block->count - c);
					if (mover.current_block->next) {
						mover.current_block = mover.current_block->next;
					}
					else {
						throw new OutOfBoundsException();
					}
				}
				else {
					return self_type(mover.current_block->array + c + i, c + i, mover.current_block);
				}
			}
		}
		self_type operator-(const int b) {
			assert(current_block->count > 0);
			int i = b, c = current_position;
			self_type mover(*this);
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
		difference_type operator-(const self_type& b) {
			int accumulator = 0;
			int c = current_position;
			self_type mover(*this);
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
		bool operator==(const self_type& a) const {
			return ptr == a.ptr;
		}
		bool operator!=(const self_type& a) const {
			return ptr != a.ptr;
		}
		// --------this--------a--------
		// if we move pointer backwards and dont impact a.block and encounter 0 => (this) is less
		bool operator<(const self_type& a) const {
			self_type mover(*this);
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
		bool operator>(const self_type& a) const {
			self_type mover(*this);
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
		bool operator<=(const self_type& a) const {
			return (*this == a || *this < a);
		}
		bool operator>=(const self_type& a) const {
			return (*this == a || *this > a);
		}
		self_type operator+=(int n) {
			for (int i = 0; i < n; i++) {
				move_next();
			}
			return *this;
		}
		self_type operator-=(int n) {
			for (int i = 0; i < n; i++) {
				move_back();
			}
			return *this;
		}
		// i'm not sure how to make random access right
		reference operator[](int j) {
			//rewind to begin??
			self_type a(*this);

			for (int i = 0; i < j; i++) {
				a.move_next();
			}
			return *a;
		}
		void swap(reference a, reference b) {
			self_type c = a;
			a = b;
			b = c;
		}
	private:
		pointer move_next() {
			if (++current_position < current_block->count) {
				return ptr = &((*current_block)[current_position]);
			}
			else if (current_block->next != 0) {
				current_block = current_block->next;
				current_position = 0;
				return ptr = &((*current_block)[current_position]);
			}
			else {
				return ptr = &((*current_block)[current_position]);
			}
		}
		pointer move_back() {
			if (--current_position > 0) {
				return ptr = &((*current_block)[current_position]);
			}
			else if (current_block->prev != 0) {
				current_block = current_block->prev;
				current_position = current_block->count-1;
				return ptr = &((*current_block)[current_position]);
			}
			else {
				return ptr = &((*current_block)[current_position]);
			}
		}
	}; // end of const_iterator class
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> reverse_const_iterator;
	//end of iterators
	block_vector() : length(0), blocks_count(0), blocks(0) {
		resize_blocks(1);
	};
	block_vector(self_type const & bv) : blocks_count(bv.blocks_count), length(bv.length) {
		resize_blocks(blocks_count);
		std::copy(&bv.blocks[0], &bv.blocks[blocks_count], blocks);
	}
	~block_vector() {
		delete[] blocks;
	}

	reference front() const {
		return blocks[0].front();
	}

	void push_back(const T & val) {
		try {
			blocks[blocks_count - 1].push_back(val);
		}
		catch (OverloadBlockException) {
			resize_blocks(blocks_count + 1);
			blocks[blocks_count - 2].next = &blocks[blocks_count - 1];
			blocks[blocks_count - 1].prev = &blocks[blocks_count - 2];
			blocks[blocks_count - 1].push_back(val);
		}
		length++;
	}

	void pop_front() {
		try {
			blocks[0].pop_front();
		}
		catch (EmptyBlockException) {
			block* newblocks = new block[blocks_count-1];
			std::copy(&blocks[1], &blocks[blocks_count], newblocks);
			newblocks[0].prev = 0;
			delete[] blocks;
			blocks = newblocks;
			blocks_count--;
		}
		catch (...) {
			throw;
		}
		length--;
	}

	void insert(const T & val, const int i) {
		// guard clause
		if (i > length || i < 0)
			throw OutOfBoundsException();

		int accumulator = 0;
		int current_block_index = 0;
		while (accumulator + blocks[current_block_index].count < i) {
			accumulator += blocks[current_block_index++].count;
		}
		int target_index = i - accumulator;
		try {
			blocks[current_block_index].insert(val, target_index);
		}
		catch (OverloadBlockException) {
			insert_new_block(current_block_index + 1);
			std::copy(&blocks[current_block_index][target_index+1],
					  &blocks[current_block_index][block::MAXCOUNT],
					  &blocks[current_block_index + 1].array[0]);
			blocks[current_block_index].count = target_index + 1;
			blocks[current_block_index+1].count = block::MAXCOUNT - target_index - 1;
			blocks[current_block_index].insert(val, target_index);
		}
		catch (...) {
			throw;
		}
		length++;
	}

	void remove_at(const int i) {
		if (i > length || i < 0)
			throw OutOfBoundsException();

		int accumulator = 0;
		int current_block_index = 0;
		while (accumulator + blocks[current_block_index].count < i) {
			accumulator += blocks[current_block_index++].count;
		}

		try {
			blocks[current_block_index].remove_at(i - accumulator);
		}
		catch (EmptyBlockException) {
			throw "NOT IMPLEMENTED";
		}
		catch (...) {
			throw;
		}
		length--;
	}

	// begin of iterator references
	iterator begin() {
		return iterator(&blocks[0].front(), 0, &blocks[0]);
	}
	iterator end() {
		return iterator(&last_block()->last(), last_block()->count, last_block());
	}
	const_iterator begin() const {
		return const_iterator(&blocks[0].front(), 0, &blocks[0]);
	}
	const_iterator end() const {
		return const_iterator(&last_block()->last(), last_block()->count, last_block());
	}
	const_iterator cbegin() {
		return const_iterator(blocks[0].front(), 0, &blocks[0]);
	}
	const_iterator cend() {
		return const_iterator(&last_block()->last(), last_block()->count, last_block());
	}
	reverse_iterator rbegin() {
		return reverse_iterator(end() - 1);
	}
	reverse_iterator rend() {
		return reverse_iterator(begin() - 1);
	}
	reverse_const_iterator rcbegin() {
		return reverse_const_iterator(end() - 1);
	}
	reverse_const_iterator rcend() {
		return reverse_const_iterator(begin() - 1);
	}
	// end of iterator references

	bool operator==(self_type& bv) {
		if (size() != bv.size())
			return false;
		bool flag = true;
		iterator p1 = begin();
		iterator p2 = bv.begin();
		for (; p1 != end() && flag; p1++, p2++) {
			if (*p1 != *p2) {
				flag = false;
			}
		}
		return flag;
	}
	bool operator!=(self_type& bv) {
		return !operator==(bv);
	}
	reference operator[](int i) {
		return begin()[i];
	}
	const_reference operator[](int i) const {
		return begin()[i];
	}
	int size() const {
		return length;
	}
	bool empty() const {
		return length == 0;
	}
private:
	block* blocks = 0;
	int blocks_count = 0;
	int length = 0;
	void resize_blocks(int newsize) {
		block* newblocks = new block[newsize];
		if (newsize < blocks_count) {
			for (int i = newsize; i < length; i++) {
				delete &blocks[i];
			}
			std::copy(&blocks[0], &blocks[newsize], newblocks);
		}
		else {
			std::copy(&blocks[0], &blocks[blocks_count], newblocks);
		}
		delete[] blocks;
		blocks = newblocks;
		blocks_count = newsize;
	}

	void insert_new_block(int pos) {
		block* newblocks = new block[blocks_count + 1];
		std::copy(&blocks[0], &blocks[pos], &newblocks[0]);
		std::copy(&blocks[pos], last_block() + 1, &newblocks[pos+1]);
		newblocks[pos].next = pos < blocks_count ? &newblocks[pos+1] : 0;
		newblocks[pos].prev = pos > 0 ? &newblocks[pos-1] : 0;
		if (pos < blocks_count) newblocks[pos+1].prev = &newblocks[pos];
		if (pos > 0) newblocks[pos-1].next = &newblocks[pos];
		delete[] blocks;
		blocks = newblocks;
		blocks_count++;
	}

	block* last_block() const {
		return &blocks[blocks_count - 1];
	}
}; // end of block_vector class

int main(int argc, const char * argv[]) {
	block_vector<int> bv;
	for (int i = 0; i < 15; i++) {
		bv.push_back(i);
	}
	std::cout << "PUSH BACK TEST COMPLETE" << std::endl;
	for (int i = 0; i < 15; i++) {
		printf("%d\n", bv[i]);
	}
	std::cout << "[]OPERATOR TEST COMPLETE" << std::endl;
	for (int i = 0; i < 12; i++) {
		bv.pop_front();
	}
	std::cout << "POP_FRONT TEST COMPLETE" << std::endl;
	std::for_each(bv.begin(), bv.end(), [](int x) {
		printf("%d\n", x);
	});
	std::cout << "BEGIN(), END() ITERATORS TEST COMPLETE" << std::endl;
	for (int i = 0; i < 20; i++) {
		bv.insert(i, 2);
	}
	for (int i = 0; i < 10; i++) {
		bv.insert(i, 5);
	}

	std::cout << "INSERT() TEST COMPLETE" << std::endl;
	std::for_each(bv.begin(), bv.end(), [](int x) {
		printf("%d\n", x);
	});


	block_vector<double> dbv;

	std::transform(bv.begin(), bv.end(), std::back_inserter(dbv), [](int x) -> double {
		return x * 0.2;
	});
	std::cout << "TRANSFORMATION TEST COMPLETE" << std::endl;
	bv[0];
}
