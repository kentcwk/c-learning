#include <iostream>
#include "my_stl_vector.h"
#include "my_stl_tree.h"

int main()
{
    rb_tree<int, int, my_identity<int>, less<int>> itree;
    cout << itree.size() << endl;
    itree.insert_unique(10);
    itree.insert_unique(7);
    itree.insert_unique(8);
    itree.insert_unique(15);
    itree.insert_unique(5);
    itree.insert_unique(6);
    itree.insert_unique(11);
    itree.insert_unique(13);
    itree.insert_unique(12);

    cout << itree.size() << endl;
    auto it = itree.begin();
    __rb_tree_base_iterator rbit;
    for (; it != itree.end(); ++it)
    {
        rbit = __rb_tree_base_iterator(it);
        cout << *it << "(" << rbit.node->color << ")" << " ";
    }
    cout << endl;
    int aa;
    cin >> aa;
    return 0;
}