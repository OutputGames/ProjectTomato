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

	T& operator[](std::size_t idx) { return vec[idx]; }
	const T& operator[](std::size_t idx) const { return vec[idx]; }

	std::vector<T> GetVector()
	{
		return vec;
	}

private:

	std::vector<T> vec;
};




#endif // COLLECTIONS_HPP
