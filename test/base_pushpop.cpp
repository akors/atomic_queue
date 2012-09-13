#include "atomic_queue.hpp"

#include <iostream>

int main()
{
    aq::atomic_queue<int> ai;

    ai.push(1);
    std::cout<<"Size is now "<<ai.size()<<'\n';

    ai.push(22);
    ai.push(330);
    ai.push(4400);
    std::cout<<"Size is now "<<ai.size()<<'\n';

    for(unsigned i = 0; i < 4; ++i)
    {
        int* p = ai.pop();
        if (!p)
        {
            std::cout<<"Qeue is empty!\n";
            continue;
        }

        std::cout<<"Popping "<<*p<<'\n';
        ai.deallocate(p);
    }

    return 0;
}
