#ifndef __MY_STL_MAP_H
#define __MY_STL_MAP_H
#include<functional>
#include "my_stl_tree.h"
template < class _Pair>
struct _Select1st : public unary_function < _Pair, typename _Pair::first_type>
{
    typename _Pair::first_type &operator()(_Pair& __x) const
    { return __x.first; }

    const typename _Pair::first_type & operator()(const _Pair& __x) const
    { return __x.first; }
};

template < class _Pair>
struct _Select2nd : public unary_function < _Pair, typename _Pair::second_type>
{
    typename _Pair::second_type& operator()(_Pair& __x) const
    { return __x.second; }
    const typename _Pair::second_type & operator()(const _Pair& __x) const
    { return __x.second; }
};
template <class Key, class T, class Compare = std::less<Key>, class Alloc = my_alloc>
class my_map
{
public:
    typedef Key key_type;
    typedef T data_type;
    typedef T mapped_type;
    typedef pair<const Key, T> value_type;
    typedef Compare key_compare;

    class value_compare : public binary_function<value_type, value_type, bool>
    {
        friend class my_map<Key, T, Compare, Alloc>;
    protected:
        Compare comp;
        value_compare(Compare c) : comp(c) {}
    public:
        bool operator()(const value_type &x, const value_type &y) const
        {
            return comp(x.first, y.first);
        }
    };

private:
    typedef rb_tree<key_type, value_type, select1st<value_type>, key_compare, Alloc> rep_type;
    rep_type t;

public:
    typedef typename rep_type::pointer pointer;
    typedef typename rep_type::const_pointer const_pointer;
    typedef typename rep_type::reference reference;
    typedef typename rep_type::const_reference const_reference;
    typedef typename rep_type::iterator iterator;
    typedef typename rep_type::const_iterator const_iterator;
    typedef typename rep_type::reverse_iterator reverse_iterator;
    typedef typename rep_type::const_reverse_iterator const_reverse_iterator;
    typedef typename rep_type::size_type size_type;
    typedef typename rep_type::difference_type difference_type;

    map() : t(Compare()) {}
    explicit map(const Compare& comp) : t(comp) {}
    template <class InputIterator>
    map(InputIterator first, InputIterator last) : t(Compare())
    {
        t.insert_unique(first, last);
    }
    template <class InputIterator>
    map(InputIterator first, InputIterator last, const Compare& comp) : t(comp)
    {
        t.insert_unique(first, last);
    }

    
};
#endif //__MY_STL_MAP_H