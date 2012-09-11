#include "atomic_queue.hpp"

#include <iostream>

int main()
{
    aq::atomic_queue<int> ai;

    ai.push(22);
    std::cout<<"Size is now "<<ai.size()<<'\n';

    ai.push(22);
    ai.push(14);
    ai.push(44);
    std::cout<<"Size is now "<<ai.size()<<'\n';
}
