#ifndef __MY_STL_VECTOR_H
#define __MY_STL_VECTOR_H
#include <memory>
#include "my_stl_alloc.h"
#include "my_stl_construct.h"
using namespace std;
template <class T, class Alloc = my_alloc>
class my_vector
{
public:
    typedef T value_type;
    typedef value_type* pointer;
    typedef value_type* iterator;
    typedef value_type& reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

protected:
    typedef simple_alloc<value_type, Alloc> data_allocator;
    iterator start;     //Ŀǰʹ�ÿռ��ͷ��
    iterator finish;    //Ŀǰʹ�ÿռ��β��
    iterator end_of_storage;    //��ʹ�ÿռ��β��

    void insert_aux(iterator position, const T& x);
    void deallocate()
    {
        if (start)
        {
            data_allocator::deallocate(start, end_of_storage - start);
        }
    }

    void fill_initialize(size_type n, const T& value)
    {
        start = allocate_and_fill(n, value);
        finish = start + n;
        end_of_storage = finish;
    }
public:
    iterator begin()
    {
        return start;
    }
    iterator end()
    {
        return finish;
    }
    size_type size() const
    {
        return size_type(finish - start);
    }
    size_type capacity() const
    {
        return size_type(end_of_storage - start);
    }
    bool empty() const {
        return start == finish;
    }
    reference operator[](size_type n)
    {
        return *(start + n);
    }

    //���캯��
    my_vector() : start(0), finish(0), end_of_storage(0) {};
    my_vector(size_type n, const T& value)
    {
        fill_initialize(n, value);
    }
    my_vector(int n, const T& value)
    {
        fill_initialize(n, value);
    }
    my_vector(long n, const T& value)
    {
        fill_initialize(n, value);
    }
    explicit my_vector(size_type n)
    {
        fill_initialize(n, T());
    }

    //��������
    ~my_vector()
    {
        destroy(start, finish);
        deallocate();
    }

    reference front()
    {
        return *begin();
    }

    reference back()
    {
        return *(end() - 1);
    }

    void push_back(const T& x)
    {
        if (finish != end_of_storage)
        {
            construct(finish, x);
            ++finish;
        }
        else
            insert_aux(end(), x);
    }

    void pop_back()
    {
        --finish;
        destroy(finish);
    }

    iterator erase(iterator position)
    {
        if (position + 1 != end())
        {
            copy(position + 1, finish, position);   //����Ԫ����ǰ��
        }
        --finish;
        destroy(finish);
        return position;
    }

    iterator erase(iterator first, iterator last)
    {
        iterator i = copy(last, finish, first);
        destroy(i, finish);
        finish = finish - (last - first);
        return first;
    }

    void resize(size_type new_size, const T& x)
    {
        if (new_size < size())
        {
            erase(begin() + new_size, end());
        }
        else
        {
            insert(end(), new_size - size(), x);
        }
    }

    void resize(size_type new_size)
    {
        resize(new_size, T());
    }
    void clear()
    {
        erase(begin(), end());
    }
    void insert(iterator position, size_type n, const T &x)
    {
        if (n != 0)
        {
            if (size_type(end_of_storage - finish) >= n)
            {
                //���ÿռ��㹻
                T x_copy = x;
                const size_type elems_after = finish - position;
                iterator old_finish = finish;
                if (elems_after > n)
                {
                    uninitialized_copy(finish - n, finish, finish);
                    finish += n;
                    copy_backward(position, old_finish - n, old_finish);
                    fill(position, position + n, x_copy);
                }
                else
                {
                    uninitialized_fill_n(finish, n - elems_after, x_copy);
                    finish += n - elems_after;
                    uninitialized_copy(position, old_finish, finish);
                    finish += elems_after;
                    fill(position, old_finish, x_copy);
                }
            }
            else
            {
                //�����ڴ治�������������ڴ�
                const size_type old_size = size();
                const size_type len = old_size + max(old_size, n);

                iterator new_start = data_allocator::allocate(len);
                iterator new_finish = new_start;

                try
                {
                    new_finish = uninitialized_copy(start, position, new_start);
                    new_finish = uninitialized_fill_n(new_start, n, x);
                    new_finish = uninitialized_copy(position, finish, new_start);
                }
                catch (...)
                {
                    destroy(new_start, new_finish);
                    data_allocator::deallocate(new_start, len);
                    throw;
                }

                //������ڴ�
                destroy(start, finish);
                deallocate();

                start = new_start;
                finish = new_finish;
                end_of_storage = new_start + len;
            }
        }
    }

protected:
    //���ÿռ䲢����
    iterator allocate_and_fill(size_type n, const T& x)
    {
        iterator result = data_allocator::allocate(n);
        uninitialized_fill_n(result, n, x);
        return result;
    }
};

template <class T, class Alloc>
void my_vector<T, Alloc>::insert_aux(iterator position, const T& x)
{
    if (finish != end_of_storage)
    {
        //���б��ÿռ�,�����һ��Ԫ�س�ʼ��һ���µ�Ԫ��
        construct(finish, *(finish - 1));
        ++finish;
        T x_copy = x;
        copy_backward(position, finish - 2, finish - 1);    //��Ŀ��λ�ÿ�ʼ�����Ų��һ��λ��copy_backward����Ӻ���ǰ�Ʊ��⸲��
        *position = x_copy;
    }
    else
    {
        //û�ñ����ڴ�
        const size_type old_size = size();
        const size_type len = old_size != 0 ? 2 * old_size : 1;

        iterator new_start = data_allocator::allocate(len);
        iterator new_finish = new_start;
        try
        {
            //��ԭ��vector���ݿ������µ�vector
            new_finish = uninitialized_copy(start, position, new_start);
            construct(new_finish, x);
            ++new_finish;
            new_finish = uninitialized_copy(position, finish, new_finish);

        }
        catch (...)
        {
        	//commit or rollback
            destroy(new_start, new_finish);
            data_allocator::deallocate(new_start, len);
            throw;
        }

        //�������ͷ�ԭvector
        destroy(begin(), end());
        deallocate();
        
        //����������ָ����vector
        start = new_start;
        finish = new_finish;
        end_of_storage = new_start + len;
    }
}
#endif //__MY_STL_VECTOR_H