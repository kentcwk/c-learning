#ifndef __MY_STL_CONSTRUCT_H
#define __MY_STL_CONSTRUCT_H

#include <new>
#include <type_traits>
#include <iterator>
using namespace std;
//构造函数
template <class T1, class T2>
inline void construct(T1 *p, const T2 &value)
{
    new (p) T1(value);
}

//析构函数
template <class ForwardIterator> inline void __destroy_aux(ForwardIterator first, ForwardIterator last, true_type);
template <class ForwardIterator> inline void __destroy_aux(ForwardIterator first, ForwardIterator last, false_type);
template <class ForwardIterator, class T> inline void __destroy(ForwardIterator first, ForwardIterator last, T*);
template<class T>
inline void destroy(T* pointer)
{
    pointer->~T();
}

template <class ForwardIterator>
inline void destroy(ForwardIterator first, ForwardIterator last)
{
    typedef typename iterator_traits<ForwardIterator>::pointer it_value_type;
    __destroy(first, last, it_value_type());
}

template <class ForwardIterator, class T>
inline void __destroy(ForwardIterator first, ForwardIterator last, T*)
{
    typedef class is_trivially_constructible<T> trivial_destructor;
    __destroy_aux(first, last, trivial_destructor());
}


template <class ForwardIterator>
inline void __destroy_aux(ForwardIterator first, ForwardIterator last, false_type)
{
    for (; first < last; ++first)
    {
        destroy(&*first);
    }
}

template <class ForwardIterator>
inline void __destroy_aux(ForwardIterator first, ForwardIterator last, true_type)
{
    //什么都不需要干
}


//针对char*的特化
inline void destroy(char*, char *) {}
inline void destroy(wchar_t*, wchar_t*) {}
#endif //__MY_STL_CONSTRUCT_H