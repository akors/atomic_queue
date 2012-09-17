atomic_queue
============

A thread-safe and lock-free implementation of the First-In-First-Out data structure pattern in C++.


1) Motivation
-------------

I would quite like to say that this class came out of the lack for a thread-safe but lock free container publicly available.
The truth is however, that I have no idea about whether there actually *is* a lack of it. Rather, I was curious about the language features provided by the recent (2011) C++ standard and I wanted to learn about lock-free programming and put my skills into action.

I have no clue if this class is actually useful, but I indend to use it in future implementations of a Logger. If you find it useful, drop me a note!


2) Usage
--------

The usage of this class is very simple:

Create an instance of this class with the default constructor, push objects to the queue with the `push()` member function and remove them from the queu with the `pop()` function. All pointers returned by `pop()` must be deallocated with the `deallocate()` member.

If the queue is empty, every call to pop() returns nullptr.

    aq::atomic_qeue_base<int> ai;
    ai.push(5);
    ai.push(7);

    int *i = ai.pop();
    assert(*i == 5);
    ai.deallocate(i);

    i = ai.pop();
    assert(*i == 7);
    ai.deallocate(i);

    i = ai.pop();
    assert(i == nullptr);



3) About thread safety
----------------------

After the call to the constructor returns, the queue is fully thread-safe. That means simultanous invocations to push() and pop() and size() may occur and produce predictable results.

The name of the class may be misleading: all these operations are thread-safe and lock-free but far from "atomic" in the classical meaning. They are still composed of multiple instructions. These instructions however are arranged in a way that no race conditions can occur.

Although the use of the size() member might be tempting, I recommend to refrain from using it. The semantics of size() are (and connot) be well defined in a multithreaded environment, because the result is only a snapshot of the queue in one point of time and may change almost instantly after invocation. Especially, do not expect `if(size() != 0) assert(pop() != nullptr)` to hold. Even if push()/pop() are not called after invoking size(), the value may still change due to finishing push()/pop() invocations.

Why is the pop() function so weird? Why doesn't it just return the object by reference? Why isn't there a seperate pop() and front() function? The reason is that 

4) Rationale
------------

### 4.1) Simplicity and minimal set of methods ###

This class may seem a little "crude": it only provides 3 methods and a basic constructor, but no typedefs or "convenience" functions. This is on purpose: I wanted to create a minimal working queue implementation without much "eye candy".

In particular, the pop() mechanism might look inconvenient: why is the value returned by a raw pointer and why does it have to be deallocated manually, with a weird member function? The reason is that this approach provides maximum performance and flexibility.

Any types that are not MoveConstructible would have to be copied when returning the value. Returning by a shared_ptr would include overhead that might not be neccessary in various use cases.
Last but not least, if a pop() is immediately following a push(), then the deallocation must wait for the push() function to finish.
If the pointer is returned to the user directly, then deallocation is likely to be delayed long after the the push() operation finishes and this makes any waits unnecessary.


### 4.2) Iterators and the use of STL algorithms ###

I would very much like decorate this class as "STL compatible", meaning that it fulfills certain container requirements. This however stands and falls with a meaningful definition of iterators, which is quite hard:

What guarantees can an iterator make? While one thread holds a pair of iterators and performs an operation on the queue, any other thread is free to push() or pop() objects at will. Any pop() operation would naturally invalidate iterators at the front and cause any ongoing operations on iterators to produce undefined behaviour.
This could be prevented by locking the front of the queue for any iterators that are in scope. This however would complicate the matter a lot and would violate the whole purpose of this class: being "lock-free".

