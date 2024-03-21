#ifndef ONLINE_COMPILE
#include "head.cpp"
#endif

#include <iostream>

void Test1()
{
    if (Solution().twoDif(1, 2) == -1)
    {
        std::cout << "用例1通过...OK" << std::endl;
    }
    else
    {
        std::cout << "用例1未通过..." << std::endl;
    }
}
void Test2()
{
    if (Solution().twoDif(-4, 1) == -5)
    {
        std::cout << "用例2通过...OK" << std::endl;
    }
    else
    {
        std::cout << "用例2未通过..." << std::endl;
    }
}

int main()
{
    Test1();
    Test2();

    return 0;
}