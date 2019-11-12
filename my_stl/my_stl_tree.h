#ifndef __MY_STL_TREE_H
#define __MY_STL_TREE_H
#include <iterator>
#include "my_stl_alloc.h"
#include "my_stl_construct.h"

typedef bool __rb_tree_color_type;
const __rb_tree_color_type __rb_tree_red = false;
const __rb_tree_color_type __rb_tree_black = true;

template<class _Ty>
struct my_identity
{	
    using type = _Ty;

    const _Ty& operator()(const _Ty& _Left) const
    {	// apply identity operator to operand
        return (_Left);
    }
};

struct __rb_tree_node_base
{
    typedef __rb_tree_color_type color_type;
    typedef __rb_tree_node_base* base_ptr;

    color_type color;       //节点颜色
    base_ptr parent;    //指向父节点的指针
    base_ptr left;
    base_ptr right;

    static base_ptr minimum(base_ptr x)
    {
        while (x->left != 0)
        {
            x = x->left;
        }
        return x;
    }

    static base_ptr maximum(base_ptr x)
    {
        while (x->right != 0)
        {
            x = x->right;
        }
        return x;
    }
};

template <class Value>
struct __rb_tree_node : public __rb_tree_node_base
{
    typedef __rb_tree_node<Value>* link_type;
    Value value_field;      //节点的值
};

struct __rb_tree_base_iterator
{
    typedef __rb_tree_node_base::base_ptr base_ptr;
    typedef bidirectional_iterator_tag iterator_category;
    typedef ptrdiff_t difference_type;

    base_ptr node;  //它用来与容器之间产生一个连结关系

    void increment()
    {
        //情形1：如果右子树不为空，下一个节点为右子树最左节点
        if (node->right != 0)
        {
            node = node->right;
            while (node->left != 0)
            {
                node = node->left;
            }
        }
        //情形2：没有右子节点，找出父节点，现行节点不是父节点的右节点为止
        else
        {
            base_ptr y = node->parent;
            while (node == y->right)
            {
                node = y;
                y = y->parent;
            }
            //情形3：当根节点没有父节点，同时没有右节点时，由于红黑树使用一个null节点来作为空节点，两者会相等。这种情况下根节点应当就是下一个节点
            //情景4：header节点的父节点是根节点，右节点是树的rightmost节点，如果根节点没有右节点的话，下面判断有可能不成立，此时node指向header为最终结果，表示下一个节点是end()
            if (node->right != y)
            {
                node = y;
            }
        }
 
    }

    void decrement()
    {
        //状况1：如果node指向header节点，header节点的父节点为根节点，根节点的父节点为header节点，以此证明该节点是否为header节点。
        //header节点的前一个节点为mostright，即为header->right
        if (node->color == __rb_tree_red &&
            node->parent->parent == node)
        {
            node = node->right;
        }
        //状况2：如果有左节点，则前一个元素既是左子树的最右节点
        else if (node->left != 0)
        {
            base_ptr y = node->left;
            while (y->right != 0)
            {
                y = y->right;
            }
            node = y;
        }
        //状况3：没有左子树，则前一个节点为自身作为有节点的第一个父节点
        else
        {
            base_ptr y = node->parent;
            while (node == y->left)
            {
                node = y;
                y = y->parent;
            }
            node = y;
        }
    }

};

inline bool operator==(const __rb_tree_base_iterator& __x,
    const __rb_tree_base_iterator& __y) {
    return __x.node == __y.node;
}

inline bool operator!=(const __rb_tree_base_iterator& __x,
    const __rb_tree_base_iterator& __y) {
    return __x.node != __y.node;
}

//RB_TREE的正规迭代器
template <class Value, class Ref, class Ptr>
struct __rb_tree_iterator : public __rb_tree_base_iterator
{
    typedef Value value_type;
    typedef Ref reference;
    typedef Ptr pointer;
    typedef __rb_tree_iterator<Value, Value&, Value*> iterator;
    typedef __rb_tree_iterator<Value, const Value&, const Value*> const_iterator;
    typedef __rb_tree_iterator< Value, Ref, Ptr> self;
    typedef __rb_tree_node<Value>* link_type;

    __rb_tree_iterator() {}
    __rb_tree_iterator(link_type x)
    {
        node = x;
    }
    __rb_tree_iterator(const iterator& it)
    {
        node = it.node;
    }

    reference operator*() const {
        return link_type(node)->value_field;
    }

    pointer operator->() const {
        return *(operator*());
    }

    //前缀++
    self& operator++()
    {
        increment();
        return *this;
    }

    //后缀++
    self operator++(int)
    {
        self tmp = *this;
        increment();
        return tmp;
    }

    //前缀--
    self& operator--()
    {
        decrement();
        return *this;
    }

    //后缀--
    self operator--(int)
    {
        self tmp = *this;
        decrement();
        return tmp;
    }
};

template <class Key, class Value, class KeyOfValue, class Compare, class Alloc = my_alloc>
class rb_tree
{
protected:
    typedef void* void_pointer;
    typedef __rb_tree_node_base* base_ptr;
    typedef __rb_tree_node<Value> rb_tree_node;
    typedef simple_alloc<rb_tree_node, Alloc> rb_tree_node_allocator;
    typedef __rb_tree_color_type color_type;

public:
    typedef Key key_type;
    typedef Value value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef rb_tree_node* link_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

protected:
    link_type get_node()
    {
        return rb_tree_node_allocator::allocate();
    }
    void put_node(link_type p)
    {
        rb_tree_node_allocator::deallocate(p);
    }

    link_type create_node(const value_type& x)
    {
        link_type tmp = get_node();
        try
        {
            construct(&tmp->value_field, x);
        }
        catch (...)
        {
            put_node(tmp);
            exit(1);
        }
        
        return tmp;
    }

    link_type clone_node(link_type x)       //复制一个节点的值和色
    {
        link_type tmp = create_node(x->value_field);
        tmp->color = x->color;
        tmp->left = 0;
        tmp->right = 0;
        return tmp;
    }

    void destroy_node(link_type p)
    {
        destroy(&p->value_field);
        put_node(p);
    }

protected:
    size_type node_count;   //记录树的大小（节点数量）
    link_type header;
    Compare key_compare;    //节点之间的简直大小比较准则，应该会是一个仿函数

    //一下三个函数用来方便取得header的成员
    link_type& root() const {
        return (link_type&)header->parent;
    }
    link_type& leftmost() const
    {
        return (link_type&)header->left;
    }
    link_type& rightmost() const {
        return (link_type&)header->right;
    }

    //以下6个函数方便取得节点x的成员
    static link_type& left(link_type x)
    {
        return (link_type&)(x->left);
    }
    static link_type& right(link_type x)
    {
        return (link_type&)(x->right);
    }
    static link_type& parent(link_type x)
    {
        return (link_type&)(x->parent);
    }
    static reference value(link_type x)
    {
        return x->value_field;
    }
    static const Key& key(link_type x)
    {
        return KeyOfValue()(value(x));
    }
    static color_type& color(link_type x)
    {
        return (color_type&)(x->color);
    }

    //以下6个函数方便取得节点x的成员,相当于重载
    static link_type& left(base_ptr x)
    {
        return (link_type&)(x->left);
    }
    static link_type& right(base_ptr x)
    {
        return (link_type&)(x->right);
    }
    static link_type& parent(base_ptr x)
    {
        return (link_type&)(x->parent);
    }
    static reference value(base_ptr x)
    {
        return ((link_type)x)->value_field;
    }
    static const Key& key(base_ptr x)
    {
        return KeyOfValue()(value(link_type(x)));
    }
    static color_type& color(base_ptr x)
    {
        return (color_type&)(link_type(x)->color);
    }

    //求极大值和极小值
    static link_type minimum(link_type x)
    {
        return (link_type) __rb_tree_node_base::minimum(x);
    }
    static link_type maximum(link_type x)
    {
        return (link_type) __rb_tree_node_base::maximum(x);
    }

public:
    typedef __rb_tree_iterator<value_type, reference, pointer> iterator;
    typedef __rb_tree_iterator<value_type, const_reference, const_pointer> const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

private:
    iterator __insert(base_ptr x, base_ptr y, const value_type& v);
    link_type __copy(link_type x, link_type p);
    void __erase(link_type x)
    {
        while (x != 0) {
            __erase(right(x));
            link_type y = left(x);
            destroy_node(x);
            x = y;
        }
    }
    void init()
    {
        header = get_node();
        color(header) = __rb_tree_red;
        root() = 0;
        leftmost() = header;
        rightmost() = header;
    }
    void clear()
    {
        __erase(root());
        leftmost() = header;
        root() = 0;
        rightmost() = header;
        node_count = 0;
    }

public:
    rb_tree(const Compare& comp = Compare()) : node_count(0), key_compare(comp)
    {
        init();
    }

    ~rb_tree()
    {
        clear();
        put_node(header);
    }

    rb_tree<Key, Value, KeyOfValue, Compare, Alloc>& operator=(const rb_tree<Key, Value, KeyOfValue, Compare, Alloc>& x);
public:
    Compare Key_comp() const {
        return key_compare;
    }
    iterator begin()
    {
        return leftmost();
    }
    iterator end()
    {
        return header;
    }
    const_iterator begin() const { return leftmost(); }
    const_iterator end() const { return header; }
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(end());
    }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const {
        return const_reverse_iterator(begin());
    }
    bool empty() const {
        return node_count == 0;
    }
    size_type size() const
    {
        return node_count;
    }
    size_type max_size() const
    {
        return size_type(-1);
    }
    iterator find(const Key &k)
    {
        link_type y = header;
        link_type x = root();

        while (x != 0)
        {
            if (!key_compare(key(x), k))
            {
                y = x;
                x = left(x);
            }
            else
            {
                x = right(x);
            }
        }
        iterator j = iterator(y);
        return (j == end() || key_compare(k, key(j->node))) ? end() : j;
    }

public:
    //插入节点，不允许重复
    pair<iterator, bool> insert_unique(const value_type& v);
    void insert_unique(const_iterator __first, const_iterator __last);
    //插入节点，允许重复
    iterator insert_equal(const value_type& v);
};



template <class Key, class Value, class KeyOfValue, class Compare, class Alloc /*= my_alloc*/>
std::pair<typename rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::iterator, bool> 
rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::insert_unique(const value_type& v)
{
    link_type y = header;
    link_type x = root();

    bool comp = true;
    while (x != 0)
    {
        y = x;
        comp = key_compare(KeyOfValue()(v), key(x));
        x = comp ? left(x) : right(x);
    }

    iterator j = iterator(y);
    //如果比父节点小
    if (comp)
    {
        //情景1：如果时最小的，没问题，直接填入
        if (j == begin())
        {
            return pair<iterator, bool>(__insert(x, y, v), true);
        }
        //j指向比父节点小的那个节点
        else
        {
            --j;
        }
    }

    //情景2：如果之前比父节点小，此时又比父节点的上一个节点大，则没有重复
    //情景3：如果之前大于等于父节点，则此时判断父节点是否比该节点小，如果小于的话也没有重复
    if (key_compare(key(j.node), KeyOfValue()(v)))
    {
        return pair<iterator, bool>(__insert(x, y, v), true);
    }

    //情景4：至此，表示新值一定和树中键值重复，那么不插入新值
    return pair<iterator, bool>(j, false);
}
template <class Key, class Value, class KeyOfValue, class Compare, class Alloc /*= my_alloc*/>
void rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::insert_unique(const_iterator __first, const_iterator __last)
{
    for (; __first != __last; ++__first)
        insert_unique(*__first);
}
template <class Key, class Value, class KeyOfValue, class Compare, class Alloc /*= my_alloc*/>
typename rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::iterator
 rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::insert_equal(const value_type& v)
{
    link_type y = header;
    link_type x = root();
    while (x != 0)
    {
        y = x;
        //遇大则往左，小于等于往右
        x = key_compare(KeyOfValue()(v), key(x)) ? left(x) : right(x);
    }

    return __insert(x, y, v);
}

inline void __rb_tree_rebalance(__rb_tree_node_base *x, __rb_tree_node_base* &root);
//真正执行插入的程序
template <class Key, class Value, class KeyOfValue, class Compare, class Alloc /*= my_alloc*/>
typename rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::iterator 
rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::__insert(base_ptr x_, base_ptr y_, const value_type& v)
{
    link_type x = (link_type)x_;
    link_type y = (link_type)y_;
    link_type z;

    //TODO:什么情况下x会!=0
    if (y == header || x != 0 || key_compare(KeyOfValue()(v), key(y)))
    {
        z = create_node(v);
        left(y) = z;    //当y为header时，使leftmost指向z
        if (y == header)
        {
            root() = z;
            rightmost() = z;
        }

        else if (y == leftmost())
        {
            leftmost() = z;
        }
    }
    else
    {
        z = create_node(v);
        right(y) = z;
        if (y == rightmost())
        {
            rightmost() = z;
        }
    }

    parent(z) = y;
    left(z) = 0;
    right(z) = 0;

    //确定节点的颜色并调整rb tree
    __rb_tree_rebalance(z, header->parent);
    ++node_count;
    return iterator(z);
}

//全局函数：调整红黑树
//重新令树形平衡

inline void __rb_tree_rotate_left(__rb_tree_node_base *x, __rb_tree_node_base* & root);
inline void __rb_tree_rotate_right(__rb_tree_node_base *x, __rb_tree_node_base* & root);
inline void __rb_tree_rebalance(__rb_tree_node_base *x, __rb_tree_node_base* &root)
{
    x->color = __rb_tree_red;
    //父节点如果使红节点，则需要调整
    while (x != root && x->parent->color == __rb_tree_red)
    {
        //父节点为祖父节点的左节点
        if (x->parent == x->parent->parent->left)
        {
            __rb_tree_node_base* y = x->parent->parent->right;  //y为伯父节点
            if (y && y->color == __rb_tree_red) //伯父节点存在且为红色
            {
                //当父节点和伯父节点均为红色时，将祖父节点的黑色沉到其两个子节点，使得两个字数黑节点深度不变。但是此时祖父节点变成了红色，需要向上检查
                x->parent->color = __rb_tree_black;
                y->color = __rb_tree_black;
                x->parent->parent->color = __rb_tree_red;
                x = x->parent->parent;
            }
            else
            {
                //无伯父节点或者伯父节点为黑色
                if (x == x->parent->right)
                {
                    //如果新增节点是父节点的右节点，则沿着父节点左旋将其转左边
                    x = x->parent;
                    __rb_tree_rotate_left(x, root);
                }

                x->parent->color = __rb_tree_black;
                x->parent->parent->color = __rb_tree_red;
                __rb_tree_rotate_right(x->parent->parent, root);
            }
        }
        else  //父节点为祖父节点的右子节点
        {
            __rb_tree_node_base *y = x->parent->parent->left;
            if (y && y->color == __rb_tree_red) //伯父节点存在且为红色
            {
                //当父节点和伯父节点均为红色时，将祖父节点的黑色沉到其两个子节点，使得两个字数黑节点深度不变。但是此时祖父节点变成了红色，需要向上检查
                x->parent->color = __rb_tree_black;
                y->color = __rb_tree_black;
                x->parent->parent->color = __rb_tree_red;
                x = x->parent->parent;
            }
            else
            {
                //无伯父节点或者伯父节点为黑色
                if (x == x->parent->left)
                {
                    //如果新增节点是父节点的左节点，则沿着父节点右旋将其转右边
                    x = x->parent;
                    __rb_tree_rotate_right(x, root);
                }

                x->parent->color = __rb_tree_black;
                x->parent->parent->color = __rb_tree_red;
                __rb_tree_rotate_left(x->parent->parent, root);
            }
        }
    }

    root->color = __rb_tree_black;
}

inline void __rb_tree_rotate_left(__rb_tree_node_base *x, __rb_tree_node_base* & root)
{
    __rb_tree_node_base *y = x->right;
    x->right = y->left;
    if (y->left != 0)
    {
        y->left->parent = x;
    }
    y->parent = x->parent;
    if (x == root)
    {
        root = y;
    }
    else if (x == x->parent->left)
    {
        x->parent->left = y;
    }
    else
    {
        x->parent->right = y;
    }

    y->left = x;
    x->parent = y;
}

inline void __rb_tree_rotate_right(__rb_tree_node_base *x, __rb_tree_node_base* & root)
{
    __rb_tree_node_base *y = x->left;
    x->left = y->right;
    if (y->right != 0)
    {
        y->right->parent = x;
    }
    y->parent = x->parent;
    if (x == root)
    {
        root = y;
    }
    else if (x == x->parent->right)
    {
        x->parent->right = y;
    }
    else
    {
        x->parent->left = y;
    }

    y->right = x;
    x->parent = y;
}



#endif //__MY_STL_TREE_H