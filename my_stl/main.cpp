#include <iostream>
#include "my_stl_vector.h"

int main()
{
    my_vector<int> kkk;
  
    for (int i = 0; i < 100; ++i)
    {
        kkk.push_back(i);
        cout << kkk.back() << " ";
        cout << kkk.size() << " " << kkk.capacity() << endl;
    }

    for (int i = 0; i < 100; ++i)
    {
        cout << kkk[i] << " ";
    }
    cout << endl;

    int aa;
    cin >> aa;
    return 0;
}