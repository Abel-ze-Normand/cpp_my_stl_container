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
	// service exceprions
	struct OverloadBlockException { };
	struct OutOfBoundsException { };
	struct EmptyBlockException { };
	class block {
	friend class block_vector;
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
		block(block* __n, block* __p) : next(__n), prev(__p) {
			array = new T[MAXCOUNT];
		};
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
				//delete &array[position];
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
			if (self_destruct) {
				if (next) {
					next->self_destruct = true;
					delete next;
				}
				return;
			}
			if (prev) {
				prev->next = next;
			}
			if (next) {
				next->prev = prev;
			}
		}
	private:
		bool self_destruct = false;
		void activate_self_destruct() {
			self_destruct = true;
		}
		void swap(block & s) {
			std::swap(array, s.array);
			std::swap(count, s.count);
			std::swap(next, s.next);
			std::swap(prev, s.prev);
		}
	};
	// container typedefs
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

		self_type& operator=(const self_type& i) {
			if (this != &i) {
				current_position = i.current_position;
				ptr = i.ptr;
				current_block = i.current_block;
			}
			return *this;
		}

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
				if (c + i >= mover.current_block->count) {
					i -= (mover.current_block->count - c);
					c = 0;
					if (mover.current_block->next) {
						mover.current_block = mover.current_block->next;
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
					accumulator += c;
					c = mover.current_block->count;
				}
				else {
					return accumulator + c - b.current_position + 1;
				}
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
			if (--current_position >= 0) {
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
		const_iterator(const const_iterator & i) : ptr(i.ptr),
									   current_position(i.current_position), current_block(i.current_block) { };
		~const_iterator() { };

		self_type& operator=(const self_type & i) {
			if (this != &i) {
				current_position = i.current_position;
				ptr = i.ptr;
				current_block = i.current_block;
			}
			return *this;
		}
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
				if (c + i >= mover.current_block->count) {
					i -= (mover.current_block->count - c);
					c = 0;
					if (mover.current_block->next) {
						mover.current_block = mover.current_block->next;
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
					if (mover.current_block->prev) {
						mover.current_block = mover.current_block->prev;
					}
					accumulator += c;
					c = mover.current_block->count;
				}
				else
					return accumulator + c - b.current_position + 1;
			}
		}
		reference operator*() const {
			return *ptr;
		}
		pointer operator->() const {
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
		reference operator[](int j) const {
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
			if (--current_position >= 0) {
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
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
	//end of iterators

	block_vector() : length(0), blocks_count(1), HEAD(new block()), TAIL(HEAD) { };
	block_vector(self_type const & bv) : blocks_count(bv.blocks_count), length(bv.length) {
		HEAD = new block(*bv.HEAD);
		block* current_bv = bv.HEAD->next;
		block* current = HEAD;
		while(current_bv) {
			current->next = new block(*current_bv);
			current->next->prev = current;
			current = current->next;
			current_bv = current_bv->next;
		}
	}
	~block_vector() {
		HEAD->activate_self_destruct();
		delete HEAD;
	}

	reference front() const {
		return HEAD->front();
	}
	void push_back(const T & val) {
		try {
			TAIL->push_back(val);
		}
		catch (OverloadBlockException) {
			TAIL->next = new block();
			TAIL->next->prev = TAIL;
			TAIL = TAIL->next;
			TAIL->push_back(val);
		}
		catch (...) {
			throw;
		}
		length++;
	}
	void pop_front() {
		try {
			HEAD->pop_front();
		}
		catch (EmptyBlockException) {
			HEAD = HEAD->next;
			delete HEAD->prev;
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
		block* current = HEAD;
		while(current->count + accumulator < i) {
			accumulator += current->count;
			current = current->next;
		}
		int target_index = i - accumulator;
		try {
			current->insert(val, target_index);
		}
		catch (OverloadBlockException) {
			current->next = new block(current->next, current);
			blocks_count++;
			if (current == TAIL)
				TAIL = current->next;
			block* reserve = current->next;
			if (reserve->next) {
				reserve->next->prev = reserve;
			}
			std::copy(&current->array[target_index], &current->last(), &reserve->array[0]);
			current->count = target_index;
			reserve->count = block::MAXCOUNT - target_index;
			current->insert(val, target_index);
		}
		catch (...) {
			throw;
		}
		length++;
	}
	void remove_at(const int i) {
		//guard clause
		if (i > length || i < 0)
			throw OutOfBoundsException();

		int accumulator = 0;
		block* current = HEAD;
		while(current->count + accumulator < i) {
			accumulator += current->count;
			current = current->next;
		}
		int target_index = i - accumulator;
		try {
			current->remove_at(target_index);
		}
		catch (EmptyBlockException) {
			if (current == HEAD)
				HEAD = HEAD->next;
			if (current == TAIL)
				TAIL = TAIL->prev;
			delete current;
			blocks_count--;
		}
		catch (...) {
			throw;
		}
		length--;
	}

	// begin of iterator references
	iterator begin() {
		return iterator(HEAD->array, 0, HEAD);
	}
	iterator end() {
		return iterator(&TAIL->last(), TAIL->count, TAIL);
	}
	const_iterator begin() const {
		return const_iterator(HEAD->array, 0, HEAD);
	}
	const_iterator end() const {
		return const_iterator(&TAIL->last(), TAIL->count, TAIL);
	}
	const_iterator cbegin() {
		return const_iterator(HEAD->array, 0, HEAD);
	}
	const_iterator cend() {
		return const_iterator(&TAIL->last(), TAIL->count, TAIL);
	}
	reverse_iterator rbegin() {
		return reverse_iterator(end());
	}
	reverse_iterator rend() {
		return reverse_iterator(begin());
	}
	const_reverse_iterator rcbegin() {
		return const_reverse_iterator(cend());
	}
	const_reverse_iterator rcend() {
		return const_reverse_iterator(cbegin());
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
	block* HEAD = 0;
	block* TAIL = 0;
	int blocks_count = 0;
	int length = 0;

	block* last_block() const {
		return TAIL;
	}
}; // end of block_vector class

int main(int argc, const char * argv[]) {
	// init
	block_vector<int> bv;

	// test1
	for (int i = 0; i < 15; i++) {
		bv.push_back(i);
	}
	std::cout << "PUSH BACK TEST COMPLETE" << std::endl;

	// test2
	for (int i = 0; i < 15; i++) {
		printf("%d ", bv[i]);
	}
	std::cout << std::endl << "[]OPERATOR TEST COMPLETE" << std::endl;

	// test3
	for (int i = 0; i < 12; i++) {
		bv.pop_front();
	}
	std::cout << std::endl << "POP_FRONT TEST COMPLETE" << std::endl;

	// test4
	std::for_each(bv.begin(), bv.end(), [](int x) {
		printf("%d ", x);
	});
	std::cout << std::endl << "BEGIN(), END() ITERATORS TEST COMPLETE" << std::endl;

	// test5
	for (int i = 0; i < 20; i++) {
		bv.insert(i, 2);
	}
	for (int i = 0; i < 10; i++) {
		bv.insert(i, 5);
	}
	std::for_each(bv.begin(), bv.end(), [](int x) {
		printf("%d ", x);
	});
	std::cout << std::endl << "INSERT() TEST COMPLETE" << std::endl;

	// test6
	for (int i = 0; i < 20; i++) {
		bv.remove_at(5);
	}
	std::for_each(bv.begin(), bv.end(), [](int x) {
		std::cout << x << " ";
	});
	std::cout << std::endl << "REMOVE_AT() TEST COMPLETE" << std::endl;

	// test7
	std::cout << std::endl;
	auto i = bv.begin();
	for (; i < bv.end(); i++) {
		std::cout << *i << " ";
	}
	std::cout << std::endl << "COMPARISON OPERATORS TEST COMPLETE" << std::endl;

	std::cout << "DEBUG" << std::endl;
	for (int i = 0; i < bv.size(); i++) std::cout << bv[i] << " ";
	std::cout << std::endl << "END OF DEBUG" << std::endl;

	// test8
	std::cout << "VALUES MUST BE EQUAL:" << std::endl;
	std::cout << "FIFTH ELEMENT: " << *(bv.begin() + 5) << " AND " << bv[5] << std::endl; // 5th element
	std::cout << "VALUES MUST BE EQUAL:" << std::endl;
	std::cout << "LAST ELEMENT: " << *(bv.end() - 1) << " AND " << bv[bv.size() - 1] << std::endl; // last element

	// test9 TODO: WHY SORT DOES NOT WORK?!
	std::sort(bv.begin(), bv.end());
	std::for_each(bv.begin(), bv.end(), [](int x) {
		std::cout << x << " ";
	});
	std::cout << std::endl << "SORT ALGORITHMS TEST COMPLETE" << std::endl;

	block_vector<double> dbv;

	std::transform(bv.begin(), bv.end(), std::back_inserter(dbv), [](int x) -> double {
		return x * 0.2;
	});
	std::for_each(dbv.begin(), dbv.end(), [](double x) {
		std::cout << x << " ";
	});
	std::cout << std::endl;
	std::cout << "TRANSFORMATION TEST COMPLETE" << std::endl;

	auto ri = bv.rbegin();
	for(; ri != bv.rend(); ++ri) {
		std::cout << *ri << " ";
	}
	std::cout << std::endl << "REVERSE ITERATOR TEST COMPLETE" << std::endl;
}
