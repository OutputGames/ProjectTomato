#if !defined(COLLECTIONS_HPP)
#define COLLECTIONS_HPP

#include "utils.h"

#define mkpair(Tv, Kv) std::make_pair(Tv, Kv)

template <typename T>
class List
{
public:

	// Nested Iterator class
	class ListIterator {
	public:
		ListIterator(typename std::vector<T>::iterator it) : it(it) {}

		// Dereference operator
		T& operator*() {
			return *it;
		}

		// Increment operators
		ListIterator& operator++() {
			++it;
			return *this;
		}

		ListIterator operator++(int) {
			ListIterator tmp = *this;
			++it;
			return tmp;
		}

		// Equality/Inequality operators
		bool operator==(const ListIterator& other) const {
			return it == other.it;
		}

		bool operator!=(const ListIterator& other) const {
			return it != other.it;
		}

	private:
		typename std::vector<T>::iterator it;
	};

	int GetCount()
	{
		return vec.size();
	}

	void Add(const T& t)
	{
		vec.push_back(t);
	}

	void Add(const List<T>& v)
	{
		vec.insert(vec.end(), v.vec.begin(), v.vec.end()); // Append the second vector
	}

	void Clear()
	{
		vec.clear();
	}

	// Function to deep copy a vector of MyClass pointers
	List<T> DeepCopy() {
		var src = vec;
		std::vector<T> dest;
		dest.reserve(src.size());
		for (const T item : src) {
			dest.push_back(T(item));
		}
		return dest;
	}


	void Remove(const T& t)
	{
		// Remove the element using erase function and iterators 
		auto it = std::find(vec.begin(), vec.end(),
			t);

		// If element is found found, erase it 
		if (it != vec.end()) {
			vec.erase(it);
		}
	}

	bool Contains(const T& t)
	{
		return std::find(vec.begin(), vec.end(), t) != vec.end();
	}

	int IndexOf(const T& t)
	{
		auto it = std::find(vec.begin(), vec.end(), t);

		// If element was found 
		if (it != vec.end())
		{

			// calculating the index 
			// of K 
			int index = it - vec.begin();
			return index;
		}
		else {
			// If the element is not 
			// present in the vector
			return -1;
		}
	}

	List() = default;

	List(T* data)
	{
		vec = std::vector<T>(data, data + (sizeof data / sizeof data[0]));
	}

	List(std::vector<T> data) : vec(data) {}

	List(std::initializer_list<T> init_list) : vec(init_list) {};

	T& operator[](std::size_t idx) { return vec[idx]; }
	const T& operator[](std::size_t idx) const { return vec[idx]; }

	std::vector<T>& GetVector()
	{ 
		return vec;
	}

	T* GetArray()
	{
		return vec.data();
	}

	ListIterator begin() {
		return ListIterator(vec.begin());
	}

	ListIterator end() {
		return ListIterator(vec.end());
	}

	~List();

private:

	std::vector<T> vec = std::vector<T>();
};

template <typename T>
List<T>::~List()
{
	vec.clear();
}

template <typename T, typename K>
class Dictionary
{
public:
	int GetCount()
	{
		return vec.size();
	}

	bool Contains(T& t)
	{
		return std::find_if(vec.begin(), vec.end(), [&t](const std::pair<T, K>& element) {
			return element.first == t;
			}) != vec.end();
	}

	void Clear()
	{
		vec.clear();
	}

	bool Contains(K k)
	{
		for (auto pair : vec)
		{
			if (pair.second == k)
				return true;
		}
		return false;
	}

	void Add(const T& t, const K& k)
	{
		vec.push_back(std::make_pair(t, k));
	}

	void Add(std::pair<T,K> pair)
	{
		vec.push_back(pair);
	}

	void Insert(T t, T k)
	{
		if (Contains(t))
		{
			
		} else
		{
			Add(t, k);
		}

	}

	int IndexOf(const T& t)
	{
		int i = 0;
		for (auto val : vec)
		{
			if (val.first == t)
				return i;

			i++;
		}

		return -1;
	}


	int IndexOf(const K& k)
	{
		int i = 0;
		for (auto val : vec)
		{
			if (val.second == k)
				return i;

			i++;
		}

		return -1;
	}
 
	void Remove(T& t)
	{
		std::pair<T,K> t2;

		bool found = false;
		for (auto pair : vec)
		{
			if (pair.first == t) {
				t2 = pair;
				found = true; break;
			}
		}

		if (!found)
			return;


		// Remove the element using erase function and iterators 
		auto it = std::find(vec.begin(), vec.end(),
			t2);

		// If element is found found, erase it 
		if (it != vec.end()) {
			vec.erase(it);

			t2.second.~K();
			t2.first.~T();

		}
	}

	void Remove(K k)
	{
		std::pair<T, K> t2;

		bool found = false;
		for (auto pair : vec)
		{
			if (pair.second == k) {
				t2 = pair;
				found = true; break;
			}
		}

		if (!found)
			return;


		// Remove the element using erase function and iterators 
		auto it = std::find(vec.begin(), vec.end(),
			t2);

		// If element is found found, erase it 
		if (it != vec.end()) {
			vec.erase(it);

			t2.second.~K();
			t2.first.~T();

		}
	}

	Dictionary() = default;

	Dictionary(std::vector<std::pair<T,K>> data) : vec(data) {}

	Dictionary(std::initializer_list<std::pair<T, K>> init_list) : vec(init_list) {};

	K& operator[](T t)
	{
		for (auto& pair : vec) {
			if (pair.first == t) {
				return pair.second;
			}
		}
		vec.push_back(std::make_pair(t, K()));
		return vec.back().second;

	}


	std::vector<std::pair<T,K>> GetVector()
	{
		return vec;
	}

	~Dictionary();

	using PairType = std::pair<T, K>;
	using ContainerType = std::vector<PairType>;

	class ListIterator {
	public:
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = PairType;
		using pointer = PairType*;
		using reference = PairType&;

		ListIterator(typename ContainerType::iterator iter) : iter_(iter) {}

		reference operator*() const { return *iter_; }
		pointer operator->() { return &(*iter_); }

		// Prefix increment
		ListIterator& operator++() {
			++iter_;
			return *this;
		}

		// Postfix increment
		ListIterator operator++(int) {
			ListIterator tmp = *this;
			++(*this);
			return tmp;
		}

		friend bool operator==(const ListIterator& a, const ListIterator& b) { return a.iter_ == b.iter_; }
		friend bool operator!=(const ListIterator& a, const ListIterator& b) { return a.iter_ != b.iter_; }

	private:
		typename ContainerType::iterator iter_;
	};

	ListIterator begin() {
		return ListIterator(vec.begin());
	}

	ListIterator end() {
		return ListIterator(vec.end());
	}

	PairType& operator[](std::size_t idx) { return vec[idx]; }
	const PairType& operator[](std::size_t idx) const { return vec[idx]; }

private:

	std::vector<std::pair<T, K>> vec = std::vector<std::pair<T, K>>();
};

template <typename T, typename K>
Dictionary<T, K>::~Dictionary()
{
	vec.clear();
}

#endif // COLLECTIONS_HPP
