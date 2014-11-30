/** Syntax Highlighting test file for C++
 *  \brief Doxygen tag highlighing
 */
#include<iostream>
#include<cstdlib>

using namespace std;

int main(void)
{
    cout << "\nHello World" << endl;
    cout << "\nUnclosed String << endl;

    // One line comment
    int a = 0;

    exit(a);
}

int add(int x, int y)
{
    return x + y;
}

void TestClass::Foo(int &nVal)
{
    nVal++;
}

// EOF
