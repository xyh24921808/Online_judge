#ifndef COMPILER_ONLINE

#include "header.cpp"

#endif

void Test1()
{
    vector<int>vc{9,7,8,9,11,2,3,84};
    int ret = Solution().NumMax(vc);
    if(ret==84)
    {
        cout<<"通过用例1,OK!"<<endl;
    }
    else 
    {
        cout<<"没有通过用例1"<<endl;
    }
}
void Test2()
{
    // 通过定义临时对象，来完成方法的调用
    vector<int>vc{1,2,3,4,5,6,7,0,8,9};
    int ret = Solution().NumMax(vc);
    if(ret==9)
    {
        cout<<"通过用例2,OK!"<<endl;
    }
    else 
    {
        cout<<"没有通过用例2"<<endl;
    }
}
int main()
{
    Test1();
    Test2();
    return 0;
}