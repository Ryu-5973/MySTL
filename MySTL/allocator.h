#ifndef MYSTL_ALLOCATOR_H_
#define MYSTL_ALLOCATOR_H_

// 这个头文件包含一个模板类 allocator，用于管理内存的分配、释放，对象的构造、析构

#include "construct.h"
#include "util.h"

namespace mystl
{

// 模板类：allocator
// 模板函数代表数据类型
template <typename T>
class allocator
{
public:
	typedef T            value_type;
	typedef T*           pointer;
	typedef const T*     const_pointer;
	typedef T&           reference;
	typedef const T&     const_reference;
	typedef size_t       size_type;
	typedef ptrdiff_t    difference_type;

public:
	static T* allocate(size_type n = 1);	// 形参默认参数

	static void deallocate(T* ptr, size_type n = 0);	// n无作用，默认形参重载出两个接口

	static void construct(T* ptr);
	static void construct(T* ptr, const T& value);
	static void construct(T* ptr, T&& value);

	template <typename... Args>
	static void construct(T* ptr, Args&& ...args);

	static void destroy(T* ptr);
  	static void destroy(T* first, T* last);
};

template <typename T>
T* allocator<T>::allocate(size_type n)
{
	if (n == 0) {
	    return nullptr;
	}
  	// 通过operator new（不是new）申请所需空间并用static_cast转换为所需类型
  	return static_cast<T*>(::operator new(n * sizeof(T)));
}


template <typename T>
void allocator<T>::deallocate(T* ptr, size_type n /*size*/)
{
  	if (ptr == nullptr) {
    	return;
   	}
  	// 通过operator delete(不是delete)释放指针
  	::operator delete(ptr);
}

template <typename T>
void allocator<T>::construct(T* ptr)
{
  	mystl::construct(ptr);
}

template <typename T>
void allocator<T>::construct(T* ptr, const T& value)
{
  	mystl::construct(ptr, value);
}

template <typename T>
void allocator<T>::construct(T* ptr, T&& value)
{
  	mystl::construct(ptr, mystl::move(value));
}

template <typename T>
template <typename... Args>
void allocator<T>::construct(T* ptr, Args&&... args)
{
  	mystl::construct(ptr, mystl::forward<Args>(args)...);
}

template <typename T>
void allocator<T>::destroy(T* ptr)
{
  	mystl::destroy(ptr);
}

template <typename T>
void allocator<T>::destroy(T* first, T* last)
{
  	mystl::destroy(first, last);
}

} // namespace mystl
#endif // !MYSTL_ALLOCATOR_H_


