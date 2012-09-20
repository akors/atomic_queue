#include "atomic_queue.hpp"

#include <thread>
#include <vector>
#include <iostream>

#include "testutils.hpp"

DECLARE_TEST("base multithreaded Push/Pop operations")


#ifndef MULTITEST_THREADCOUNT
#    define MULTITEST_THREADCOUNT 64
#endif

#ifndef MULTITEST_PUSHCOUNT
#    define MULTITEST_PUSHCOUNT 256
#endif

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


int main()
{
    std::vector<std::thread> threadvec;
    aq::atomic_queue_base<Obj> queue;
    Obj* p;

    std::cout<<"\n--- Launching threads for simultanous push ---\n";
    for(unsigned ti = 0; ti < MULTITEST_THREADCOUNT; ++ti)
        threadvec.push_back(std::thread(
        [&queue, ti]{
            for(unsigned oi = 0; oi < MULTITEST_PUSHCOUNT; ++oi)
                queue.push_back(Obj(ti, oi));
            std::cout<<'#';
        }
        ));

    // wait for all threads, delete the thread vectors
    for(auto& t: threadvec)
        t.join();
    std::cout<<std::endl;

    TEST_ASSERT(queue.size() ==  MULTITEST_PUSHCOUNT * MULTITEST_THREADCOUNT);

    std::cout<<"\n--- Checking queue objects after calculation ---\n";
    while ((p = queue.pop_front()))
    {
        TEST_ASSERT(p->data_ == Obj::calculate_data(p->id_, p->idx_));
        queue.deallocate(p);
    }

    threadvec.clear();

    std::cout<<"\n--- Launching threads for simultanous push and pop ---\n";
    for(unsigned ti = 0; ti < MULTITEST_THREADCOUNT; ++ti)
        threadvec.push_back(std::thread(
        [&queue, ti]{
            for(unsigned oi = 0; oi < MULTITEST_PUSHCOUNT; ++oi)
                queue.push_back(Obj(ti, oi));
            std::cout<<'#';
        }
        ));

    std::size_t popcount = 0;
    while (popcount != MULTITEST_PUSHCOUNT * MULTITEST_THREADCOUNT)
    {
        p = queue.pop_front();
        if(!p)
        {
            std::this_thread::yield();
            continue;
        }

        TEST_ASSERT(p->data_ == Obj::calculate_data(p->id_, p->idx_));
        queue.deallocate(p);
        ++popcount;
    }

    TEST_ASSERT(queue.size() == 0);

    // wait for all threads
    for(auto& t: threadvec)
        t.join();

    std::cout<<"\n --- All threads joined ---\n";

    return CONCLUDE_TEST();
}
