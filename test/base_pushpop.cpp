#include "atomic_queue.hpp"

#include <iostream>

#include "testutils.hpp"

DECLARE_TEST("base Push/Pop operations")


void test_int()
{
    std::cout<<" === Testing integer push/pop ==="<<'\n';

    aq::atomic_queue<int> ai;

    ai.push_back(1);
    ai.push_back(22);
    ai.push_back(330);
    ai.push_back(4400);
    std::cout<<"Size is now "<<ai.size()<<'\n';
    TEST_ASSERT(ai.size() == 4);

    int* p;
    while ((p = ai.pop_front()))
    {
        std::cout<<"Popping "<<*p<<'\n';
        ai.deallocate(p);
    }

    std::cout<<"Size is now "<<ai.size()<<'\n';
    TEST_ASSERT(ai.size() == 0);
}

struct Obj {
    int id_;
    std::size_t idx_;
    int data_;

    Obj(int id, std::size_t idx)
        : id_(id), idx_(idx), data_(calculate_data(id_, idx_))
    { }


    static int calculate_data(int id, std::size_t idx)
    {
        return id ^ idx;
    }
};

const std::size_t NUM_PUSHES = 0xFF;

void test_obj()
{
    std::cout<<" === Testing Object push/pop ==="<<'\n';

    int id = 0xFF;
    std::size_t idx;

    aq::atomic_queue<Obj> ao;

    TEST_ASSERT(ao.pop_front() == nullptr);
    TEST_ASSERT(ao.size() == 0);

    std::cout<<"Pushing "<<NUM_PUSHES<<" objects.\n";

    for (idx = 0x00; idx < (0x00+NUM_PUSHES); ++idx)
        ao.push_back(Obj(id, idx));

    std::cout<<"Size is now "<<ao.size()<<'\n';
    TEST_ASSERT(ao.size() == NUM_PUSHES);

    idx = 0x00;

    Obj* p;
    while ((p = ao.pop_front()))
    {
        TEST_ASSERT(p->data_ == Obj::calculate_data(id, idx));
        ao.deallocate(p);
        ++idx;
    }

    TEST_ASSERT(ao.size() == 0);
}

int main()
{
    test_int();
    std::cout<<std::endl;
    test_obj();

    return CONCLUDE_TEST();
}
