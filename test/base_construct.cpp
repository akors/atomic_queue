#include "atomic_queue.hpp"

#include <iostream>

#include "testutils.hpp"

DECLARE_TEST("Construction of objects")


struct Obj
{
    Obj(int) : cons(INT_CONSTRUCTED)
    {
        std::cout<<"I was int-constructed!\n";
    }

    Obj(const Obj&) : cons(COPY_CONSTRUCTED)
    {
        std::cout<<"I was copy-constructed!\n";
    }

    Obj(Obj&&) : cons(MOVE_CONSTRUCTED)
    {
        std::cout<<"I was move-constructed!\n";
    }

    enum constructed_t {
        INT_CONSTRUCTED,
        COPY_CONSTRUCTED,
        MOVE_CONSTRUCTED
    } cons;
};

int main()
{
    aq::atomic_queue_base<Obj> ao;

    Obj o(33);
    ao.push_back(o);
    ao.push_back(std::move(Obj(22)));
#if !(defined(_MSC_VER) && _MSC_VER <= 1700)
    ao.emplace_back(11);
#endif

    Obj *op;

    op = ao.pop_front();
    TEST_ASSERT(op->cons == Obj::COPY_CONSTRUCTED); ao.deallocate(op);
    op = ao.pop_front();
    TEST_ASSERT(op->cons == Obj::MOVE_CONSTRUCTED); ao.deallocate(op);
#if !(defined(_MSC_VER) && _MSC_VER <= 1700)
    op = ao.pop_front();
    TEST_ASSERT(op->cons == Obj::INT_CONSTRUCTED); ao.deallocate(op);
#endif

    return CONCLUDE_TEST();
}
