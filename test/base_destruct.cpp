#include "atomic_queue.hpp"

#include <iostream>

#include "testutils.hpp"


DECLARE_TEST("base Construction/Destruction")

struct Obj {
    int id_;
    std::size_t idx_;
    int data_;

    Obj(const Obj& other)
        : id_(other.id_), idx_(other.idx_), data_(other.data_),
        construct_counter_(other.construct_counter_)
    {
        ++*construct_counter_;
    }

    Obj(int id, std::size_t idx, std::size_t* ccounter)
        : id_(id), idx_(idx), data_(calculate_data(id_, idx_)),
        construct_counter_(ccounter)
    {
        ++*construct_counter_;
    }

    ~Obj()
    {
        TEST_ASSERT(data_ == calculate_data(id_, idx_));
        --*construct_counter_;
    }

    static int calculate_data(int id, std::size_t idx)
    {
        return id ^ idx;
    }
private:
    std::size_t* construct_counter_;
};


const std::size_t NUM_PUSHES = 0xFF;

int main()
{
    int id = 0xFF;
    std::size_t construct_counter = 0;

    std::cout<<" === Testing atomic_queue destructor ==="<<'\n';
    {
        aq::atomic_queue<Obj> ao;

        std::cout<<"Pushing "<<NUM_PUSHES<<" objects.\n";
        for (std::size_t idx = 0x00; idx < (0x00+NUM_PUSHES); ++idx)
            ao.push(Obj(id, idx, &construct_counter));

        std::cout<<"Number of constructions: "<<construct_counter<<std::endl;
        std::cout<<"Destructing the whole queue object.\n";
    }

    std::cout<<"Number of constructions: "<<construct_counter<<'\n'<<'\n';
    
    std::cout<<" === Testing  destructor ==="<<'\n';
    aq::atomic_queue<Obj> ao;

    std::cout<<"Pushing "<<NUM_PUSHES<<" objects.\n";
    for (std::size_t idx = 0x00; idx < (0x00+NUM_PUSHES); ++idx)
        ao.push(Obj(id, idx, &construct_counter));

    std::cout<<"Number of constructions: "<<construct_counter<<std::endl;

    Obj* p;
    while (p = ao.pop())
    {
        ao.deallocate(p);
    }
    std::cout<<"Number of constructions: "<<construct_counter<<std::endl;

    return CONCLUDE_TEST();
}
