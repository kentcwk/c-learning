#ifndef __MY_STL_ALLOC_H
#define __MY_STL_ALLOC_H

#include <iostream>
#define __THROW_BAD_ALLOC std::cerr << "out of memory" << std::endl; exit(1)

template<class T, class Alloc>
class simple_alloc {
public:
    static T *allocate(size_t n)
    {
        return 0 == n ? 0 : (T*)Alloc::allocate(n * sizeof(T));
    }
    static T *allocate(void)
    {
        return (T*)Alloc::allocate(sizeof(T));
    }
    static void deallocate(T *p, size_t n)
    {
        if (0 != n)
        {
            Alloc::deallocate(p, n * sizeof(T));
        }
    }
    static void deallocate(T *p)
    {
        Alloc::deallocate(p, sizeof(T));
    }
};

//第一级配置器
template <int inst>
class __malloc_alloc_template
{
private:
    static void *oom_malloc(size_t);
    static void *oom_realloc(void *, size_t);
    static void(*__malloc_alloc_oom_handler)();
public:
    static void *allocate(size_t n)
    {
        void *result = malloc(n);
        if (0 == result)
        {
            result = oom_malloc(n);
        }
        return result;
    }

    static void deallocate(void *p, size_t)
    {
        free(p);
    }

    static void *reallocate(void *p, size_t, size_t new_sz)
    {
        void *result = realloc(p, new_sz);
        if (0 == result)
        {
            result = oom_realloc(p, new_sz);
        }
        return result;
    }

    static void(*set_malloc_handler(void(*f)()))()
    {
        void(*old)() = __malloc_alloc_oom_handler;
        __malloc_alloc_oom_handler = f;
        return (old);
    }
};

template <int inst>
void(*__malloc_alloc_template<inst>::__malloc_alloc_oom_handler)() = 0;
template <int inst>
void * __malloc_alloc_template<inst>::oom_malloc(size_t n)
{
    void(*my_malloc_handler)();
    void *result;
    for (;;)
    {
        my_malloc_handler = __malloc_alloc_oom_handler;
        if (0 == my_malloc_handler)
        {
            __THROW_BAD_ALLOC;
        }

        (*my_malloc_handler)();
        result = malloc(n);
        if (result)
        {
            return (result);
        }
    }
}

template <int inst>
void *__malloc_alloc_template<inst>::oom_realloc(void *p, size_t n)
{
    void(*my_malloc_handler)();
    void *result;
    for (;;)
    {
        my_malloc_handler = __malloc_alloc_oom_handler;
        if (0 == my_malloc_handler)
        {
            __THROW_BAD_ALLOC;
        }

        (*my_malloc_handler)();
        result = realloc(p, n);
        if (result)
        {
            return (result);
        }
    }
}

typedef __malloc_alloc_template<0> malloc_alloc;

//第二级配置器
enum {__ALIGN = 8};
enum {__MAX_BYTES = 128};
enum { __NFREELISTS = __MAX_BYTES / __ALIGN};

template <bool threads, int inst>
class __default_alloc_template
{
private:
    static size_t ROUND_UP(size_t bytes)
    {
        return (((bytes)+__ALIGN - 1) & ~(__ALIGN - 1));
    }
private:
    union obj
    {
        union obj * free_list_link;
        char client_data[1];
    };

private:
    static obj * volatile free_list[__NFREELISTS];
    static size_t FREELIST_INDEX(size_t bytes)
    {
        return (((bytes)+__ALIGN - 1) / __ALIGN - 1);
    }

    static void *refill(size_t n);
    static char *chunk_alloc(size_t size, int &nobjs);

    static char *start_free;
    static char *end_free;
    static size_t heap_size;

public:
    static void *allocate(size_t n)
    {
        obj * volatile * my_free_list;
        obj * result;
        if (n > (size_t)__MAX_BYTES)
        {
            return (malloc_alloc::allocate(n));
        }

        my_free_list = free_list + FREELIST_INDEX(n);
        result = *my_free_list;
        if (0 == result)
        {
            void *r = refill(ROUND_UP(n));
            return r;
        }

        *my_free_list = result->free_list_link;
        return (result);
    }
    static void deallocate(void *p, size_t n)
    {
        obj *q = (obj *)p;
        obj * volatile * my_free_list;
        if (n > (size_t)__MAX_BYTES)
        {
            malloc_alloc::deallocate(p, n);
            return;
        }

        my_free_list = free_list + FREELIST_INDEX(n);
        q->free_list_link = *my_free_list;
        *my_free_list = q;
    }
    static void *reallocate(void *p, size_t old_sz, size_t new_sz);
};

template <bool threads, int inst>
char *__default_alloc_template<threads, inst>::start_free = 0;
template <bool threads, int inst>
char *__default_alloc_template<threads, inst>::end_free = 0;
template <bool threads, int inst>
size_t __default_alloc_template<threads, inst>::heap_size = 0;

template <bool threads, int inst>
typename __default_alloc_template<threads, inst>::obj * volatile __default_alloc_template<threads, inst>::free_list[__NFREELISTS] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};

//每次尝试申请20个空间，将多余的填入free list中
template <bool threads, int inst>
void* __default_alloc_template<threads, inst>::refill(size_t n)
{
    int nobjs = 20;
    char * chunk = chunk_alloc(n, nobjs);   //内部保证至少返回1个
    obj* volatile * my_free_list;
    obj * result;
    obj * current_obj, *next_obj;
    int i;

    if (1 == nobjs)
    {
        return (chunk);
    }

    my_free_list = free_list + FREELIST_INDEX(n);

    result = (obj *)chunk;
    *my_free_list = next_obj = (obj *)(chunk + n);
    for (i = 1;; ++i)
    {
        current_obj = next_obj;
        next_obj = (obj *)((char*)next_obj + n);
        if (nobjs - 1 == i)
        {
            current_obj->free_list_link = 0;
            break;
        }
        else
        {
            current_obj->free_list_link = next_obj;
        }
    }
    return (result);
}

template <bool threads, int inst>
char* __default_alloc_template<threads, inst>::chunk_alloc(size_t size, int & nobjs)
{
    char *result;
    size_t total_bytes = size * nobjs;
    size_t bytes_left = end_free - start_free;

    if (bytes_left >= total_bytes)
    {
        //内存池满足需求
        result = start_free;
        start_free += total_bytes;
        return result;
    }
    else if (bytes_left >= size)
    {
        //至少满足一块
        nobjs = bytes_left / size;
        total_bytes = size * nobjs;
        result = start_free;
        start_free += total_bytes;
        return (result);
    }
    else
    {
        //一块都不满足
        //需要重新申请内存到内存池，申请量为当前申请量的两倍再加上累计申请总量的十六分之一
        size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);

        //将碎片直接放入对应的free list，避免浪费
        if (bytes_left > 0)
        {
            obj * volatile * my_free_list = free_list + FREELIST_INDEX(bytes_left);
            ((obj *)start_free)->free_list_link = *my_free_list;
            *my_free_list = (obj *)start_free;
        }

        //申请heap空间补充内存池
        start_free = (char*)malloc(bytes_to_get);
        if (0 == start_free)
        {
            //heap空间不足，malloc失败
            int i;
            obj * volatile * my_free_list, *p;
            //试着从较大的free list中找到是否有可以使用的内存块
            for (i = size; i < __MAX_BYTES; i += __ALIGN)
            {
                my_free_list = free_list + FREELIST_INDEX(i);
                p = *my_free_list;
                if (0 != p)
                {
                    *my_free_list = p->free_list_link;
                    start_free = (char *)p;
                    end_free = start_free + i;
                    //递归调用自己，因为现在已经有一小块内存了，重新进行判断
                    return (chunk_alloc(size, nobjs));
                }
            }

            //没找到内存,交由第一级配置器申请内存。要么申请到内存，要么由之处理异常
            end_free = 0;
            start_free = (char *)malloc_alloc::allocate(bytes_to_get);
            
        }

        heap_size += bytes_to_get;
        end_free = start_free + bytes_to_get;
        return (chunk_alloc(size, nobjs));  //现在有内存了，可以重新检查分配了
    }
}

typedef __default_alloc_template<0, 0> my_alloc;

#endif //__MY_STL_ALLOC_H