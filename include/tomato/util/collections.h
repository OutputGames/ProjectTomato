#if !defined(COLLECTIONS_HPP)
#define COLLECTIONS_HPP

#include "utils.h"

template <typename T>
class List
{
public:
	int GetCount()
	{
		return vec.size();
	}

	void Add(const T& t)
	{
		vec.push_back(t);
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

	List(std::vector<T> data)
	{
		vec = data;
	}

	T& operator[](std::size_t idx) { return vec[idx]; }
	const T& operator[](std::size_t idx) const { return vec[idx]; }

	std::vector<T> GetVector()
	{ 
		return vec;
	}

	T* GetArray()
	{
		return vec.data();
	}

private:

	std::vector<T> vec = std::vector<T>();
};

template <typename T, typename K>
class Dictionary
{
public:
	int GetCount()
	{
		return vec.size();
	}

	bool Contains(const T& t)
	{
		return vec.count(t);
	}

	void Add(const T& t, const K& k)
	{
		vec.insert(t, k);
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

	Dictionary() = default;

	Dictionary(std::map<T,K> data)
	{
		vec = data;
	}

	K& operator[](T idx) { return vec[idx]; }
	const K& operator[](T idx) const { return vec[idx]; }

	std::map<T,K> GetMap()
	{
		return vec;
	}


private:

	std::map<T, K> vec = std::map<T, K>();
};

#endif // COLLECTIONS_HPP
