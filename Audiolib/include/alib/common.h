#pragma once

#include <stdio.h>

#define ALIB_DECLARE_PRIV \
	struct Priv;          \
	Priv* p
#define ALIB_DECLARE_SHARED \
	struct Priv;            \
	std::shared_ptr<Priv> p
#define ALIB_DECLARE_IMPSHARED \
	struct Priv;               \
	alib::CowPtr<Priv> p
#define ALIB_WARN(...) fprintf(stderr, __VA_ARGS__)
#define ALIB_ERROR(...) fprintf(stderr, __VA_ARGS__)
#define ALIB_RETURN_ERROR(...)   \
	do {                         \
		ALIB_ERROR(__VA_ARGS__); \
		return;                  \
	} while (0)

#include <memory>

namespace alib {

class NonCopyable {
protected:
	NonCopyable() { }
	~NonCopyable() { }

private:
	NonCopyable(const NonCopyable&);
	NonCopyable& operator=(const NonCopyable&);
};

template <class T>
class CowPtr {
	std::shared_ptr<T> data;

public:
	CowPtr(T* t)
		: data(t)
	{
	}
	CowPtr(const std::shared_ptr<T>& refptr)
		: data(refptr)
	{
	}
	CowPtr(const CowPtr& cowptr)
		: data(cowptr.data)
	{
	}
	CowPtr& operator=(const CowPtr& rhs)
	{
		data = rhs.data;
		return *this;
	}
	~CowPtr() { }
	const T& operator*() const { return *data; }
	T& operator*()
	{
		detach();
		return *data;
	}
	const T* operator->() const { return data.operator->(); }
	T* operator->()
	{
		detach();
		return data.operator->();
	}
	void detach()
	{
		T* t = data.get();
		if (!(t == 0 || data.unique())) {
			data = std::shared_ptr<T>(new T(*t));
		}
	}
};

// Convenience function
static inline const char* copyString(const char* string)
{
	int length = 0;

	if (!string)
		return 0;

	while (length++ < 4095)
		if (!*(string + length))
			break;

	char* copy = new char[length + 1];

	for (int i = 0; i < length; ++i)
		copy[i] = string[i];

	copy[length] = 0;

	return copy;
}

}

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif
