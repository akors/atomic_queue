atomic_queue
============

A thread-safe and lock-free implementation of the First-In-First-Out data structure pattern in C++.


1) Motivation
-------------

I would quite like to say that this class came out of the lack for a thread-safe but lock free container publicly available.
The truth is however, that I have no idea about whether there actually *is* a lack of it. Rather, I was curious about the language features provided by the recent (2011) C++ standard and I wanted to learn about lock-free programming and put my skills into action.

I have no clue if this class is actually useful, but I indend to use it in future implementations of a Logger. If you find it useful, drop me a note!

2) Compilers
------------

This class compiles with the following compilers:

  * Clang >= 3.1, with -stdlib=libc++
  * GCC => 4.7, with -D_GLIBCXX_USE_SCHED_YIELD
  * Visual Studio >= 2012 (_MSC_VER >= 17.00)

It should be noted that MSVC denied me some of the C++11 features I wanted to use. Those were: right angle brackets, uniform initialization syntax, noexcept specification. I want them. Give them to me! Now!

3) Usage
--------

The usage of this class is very simple:

Create an instance of this class with the default constructor, push objects to the queue with the `push_back()` member function and remove them from the queu with the `pop_front()` function. All pointers returned by `pop_front()` must be deallocated with the `deallocate()` member.

If the queue is empty, every call to pop_front() returns nullptr.

    aq::atomic_qeue_base<int> ai;
    ai.push_back(5);
    ai.push_back(7);

    int *i = ai.pop_front();
    assert(*i == 5);
    ai.deallocate(i);

    i = ai.pop_front();
    assert(*i == 7);
    ai.deallocate(i);

    i = ai.pop_front();
    assert(i == nullptr);


4) About thread safety
----------------------

After the call to the constructor returns, the queue is fully thread-safe. That means simultanous invocations to push_back() and pop_front() and size() may occur and produce predictable results.

The name of the class may be misleading: all these operations are thread-safe and lock-free but far from "atomic" in the classical meaning. They are still composed of multiple instructions. These instructions however are arranged in a way that no race conditions can occur.

Although the use of the size() member might be tempting, I recommend to refrain from using it. The semantics of size() are (and connot) be well defined in a multithreaded environment, because the result is only a snapshot of the queue in one point of time and may change almost instantly after invocation. Especially, do not expect `if(size() != 0) assert(pop_front() != nullptr)` to hold. Even if push_back()/pop_front() are not called after invoking size(), the value may still change due to finishing push_back()/pop_front() invocations.

5) Rationale
------------

### 5.1) Simplicity and minimal set of methods ###

This class may seem a little "crude": it only provides 3 methods and a basic constructor, but no typedefs or "convenience" functions. This is on purpose: I wanted to create a minimal working queue implementation without much "eye candy".

In particular, the pop_front() mechanism might look inconvenient: Why is the value returned by a raw pointer and why does it have to be deallocated manually? Why doesn't it just return the object by reference? Why isn't there a seperate pop() and front() function? The reason is that it not sensible to return a reference to the front of the queue because the object could be popped and destroyed immediately, and you would end up with a dangling reference.

Also, if a pop_front() is immediately following a push_back(), then the deallocation must wait for the push_back() function to finish.
If the pointer is returned to the user directly, then deallocation is likely to be delayed long after the the push_back() operation finishes and this makes any waits unnecessary.

An alternative approach would be to return the object by value, but that would either require the type to be CopyConstructible (and then they would have to be copied!) or MoveConstructible, which is an unnecessary limitation.

Another approach would be to return a shared_ptr, but that  would include overhead that might not be neccessary in various use cases and also introduce additional header dependencies and increase compilation time.


### 5.2) Iterators and the use of STL algorithms ###

I would very much like decorate this class as "STL compatible", meaning that it fulfills certain container requirements. This however stands and falls with a meaningful definition of iterators, which is quite hard:

What guarantees can an iterator make? While one thread holds a pair of iterators and performs an operation on the queue, any other thread is free to push_back() or pop_front() objects at will. Any pop_front() operation would naturally invalidate iterators at the front and cause any ongoing operations on iterators to produce undefined behaviour.
This could be prevented by locking the front of the queue for any iterators that are in scope. This however would complicate the matter a lot and would violate the whole purpose of this class: being "lock-free".

### 5.3) Derived classes ###

The class is called atomic_queue_base for a reason: it is just the *base* of an atomic queue. This means that functionality is very limited and slightly inconvenient (see 5.1). This allows a clear and efficient implementation of the base functionality.
I could imagine that for a "real-world" use case, it would be interesting to extend this class. One could introduce some sort of locking mechanism or define a meaningful set of iterator types which would make this class viable for STL compatibility.
So why don't you go ahead and implement a class that derives from this one? I'm counting on you! Just don't forget to drop a pull request.





