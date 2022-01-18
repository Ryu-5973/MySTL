#ifndef MYSTL_CONSTRUCT_H_
#define MYSTL_CONSTRUCT_H_

// 这个头文件包含两个函数 construct，destroy
// construct : 负责对象的构造
// destroy   : 负责对象的析构

#include <new>

#include "type_traits.h"
#include "iterator.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)  // unused parameter
#endif // _MSC_VER

namespace mystl
{

// construct 构造对象

template <typename Ty>
void construct(Ty* ptr)
{
	// placement new 调用Ty()
  	::new ((void*)ptr) Ty();
}

template <typename Ty1, typename Ty2>
void construct(Ty1* ptr, const Ty2& value)
{
	// placement new 调用Ty1(value)
  	::new ((void*)ptr) Ty1(value);
}

template <typename Ty, typename... Args>
void construct(Ty* ptr, Args&&... args)
{
	// placement new 调用Ty(args...)
  	::new ((void*)ptr) Ty(mystl::forward<Args>(args)...);
}

// destroy 将对象析构


template <typename ForwardIter>
void __destroy(ForwardIter , ForwardIter , std::true_type) {}

template <typename ForwardIter>
void __destroy(ForwardIter first, ForwardIter last, std::false_type)
{
  	for (; first != last; ++first) {
    	destroy(&*first);
    }
}


// destroy 发生在operator delete 之前，调用析构函数
// destroy 的第一个版本，接受一个指针
template <typename Ty>
void destroy(Ty* pointer)
{
	// is_trivially_destructible<T>  <type_traits>
	// 用于检查T（Traits）是否是普通可破坏类型
	// 返回一个布尔变量表示T是否可轻易破坏
	if(pointer != nullptr){
  		pointer->~Ty();
	}
	// destroy_one(pointer, std::is_trivially_destructible<Ty>::value);
	// destroy_one(pointer, std::is_trivially_destructible<Ty>{});
	
}

// 第二个版本，接受两个迭代其
template <typename ForwardIter>
void destroy(ForwardIter first, ForwardIter last)
{
	// 对两个迭代器之间的所有元素进行销毁
  	__destroy(first, last, std::is_trivially_destructible<typename iterator_traits<ForwardIter>::value_type>{});
}

// 第二个版本对char*和wchar_t*的特化
void destroy(char*, char*);
void destroy(wchar_t*, wchar_t*);


} // namespace mystl

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // !MYTINYSTL_CONSTRUCT_H_


