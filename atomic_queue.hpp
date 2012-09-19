/*
* Copyright (c) 2012 Alexander Korsunsky <fat.lobyte9@gmail.com>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS
* IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ATOMIC_QUEUE_HPP_INCLUDED
#define ATOMIC_QUEUE_HPP_INCLUDED

#include <cstddef>
#include <memory>
#include <atomic>
#include <thread>

// At the time of writing, MSVC didn't know noexcept
#if defined(_MSC_VER) && _MSC_VER <= 1700
#   define noexcept throw()
#endif

namespace aq {

namespace detail {

/** A node in the linked list. */
template <typename T>
struct node
{
    T t /**< Value */;
    std::atomic<node<T>*> next /**< Next node in list */;
};


/** A thread-safe and lock-free queue container.
*
* Use this class as a queue where multiple threads can read from and write
* to at the same time.
* To add elements to the queue, use the push_back() function.
* To remove and retrieve elements, use the pop_front() function, but remember to
* deallocate the value with deallocate() after using it.
*
* @tparam T Type of the objects this queue will hold.
* @tparam Allocator Allocator type
*/
template <typename T, typename Allocator = std::allocator<T> >
class atomic_queue_base
{
public:

    /** Construct an empty qeue.
    *
    * @param alc Allocator object that is to be used for memory
    * allocation/deallocation.
    *
    * @note The queue is thread-safe after this function has returned.
    */
    atomic_queue_base(const Allocator& alc = Allocator()) noexcept
        : size_(0u), front_(nullptr), back_(nullptr),
        alc_(alc)
    { }


    /** Destructor.
    *
    * @note The queue is thread-safe before this function is invoked.
    */
    ~atomic_queue_base() noexcept
    {
        node<T>* fr = front_;

        while(fr)
        {
            node<T>* next = fr->next;
            NodeAllocatorTraits::destroy(alc_, fr);
            NodeAllocatorTraits::deallocate(alc_, fr, 1);
            fr = next;
        }

    }


    /** Push object into the queue by copying it.
    *
    * @param t Object you want to push into the queue. Requires T to be
    * CopyConstructible.
    * @throws Any exceptions thrown by the copy constructor of the object.
    *
    * @note This function is Thread-safe, lock-free and wait-free.
    */
    void push_back(const T& t)
    {
        auto new_node = NodeAllocatorTraits::allocate(alc_, 1);

        try {
            ValueAllocator alc(alc_);
            ValueAllocatorTraits::construct(
               alc,
               &new_node->t, t
            );
        } catch(...)
        {
            NodeAllocatorTraits::deallocate(alc_, new_node, 1);
            throw;
        }
        new_node->next = nullptr;

        push_node(new_node);
    }

    /** Push object into the queue by moving it.
    *
    * @param t Object you want to push into the queue. Requires T to be
    * MoveConstructible.
    * @throws Any exceptions thrown by the move constructor of the object.
    *
    * @note This function is Thread-safe, lock-free and wait-free.
    */
    void push_back(T&& t)
    {
        auto new_node = NodeAllocatorTraits::allocate(alc_, 1);

        try {
            ValueAllocator alc(alc_);
            ValueAllocatorTraits::construct(
               alc, &new_node->t, std::move(t)
            );
        } catch(...)
        {
            NodeAllocatorTraits::deallocate(alc_, new_node, 1);
            throw;
        }
        new_node->next = nullptr;

        push_node(new_node);
    }

// I WANT C++11!!! NOW!!!
#if !(defined(_MSC_VER) && _MSC_VER <= 1700)

    /** Create and push object into the queue.
    *
    * @param args... Arguments to the objects constructor
    * @throws Any exceptions thrown by the constructor of the object.
    *
    * @note This function is Thread-safe, lock-free and wait-free.
    */
    template<typename... Args>
    void emplace_back(Args&&... args)
    {
        auto new_node = NodeAllocatorTraits::allocate(alc_, 1);

        try {
            ValueAllocator alc(alc_);
            ValueAllocatorTraits::construct(
               alc, &new_node->t, std::forward<Args>(args)...
            );
        } catch(...)
        {
            NodeAllocatorTraits::deallocate(alc_, new_node, 1);
            throw;
        }
        new_node->next = nullptr;

        push_node(new_node);
    }
#endif


    /** Pop object from the queue.
    *
    * This function removes an object from the front of the queue and returns
    * a pointer to it. Do not delete the object manually!
    *
    * To delete an object returned by this function, use the deallocate()
    * member function.
    *
    * @return A pointer to the object that was removed from the queue.
    *
    * @note This function is Thread-safe, lock-free but not wait-free.
    */
    T* pop_front() noexcept
    {
        node<T>* old_front = front_;
        node<T>* new_front;

        do {
            if (!old_front) return nullptr; // nothing to pop
            new_front = old_front->next;
        } while(!front_.compare_exchange_weak(old_front, new_front));

        --size_;

        // if the old front was also the back, the queue is now empty.
        new_front = old_front;
        if(back_.compare_exchange_strong(new_front, nullptr))
            old_front->next = old_front;

        return reinterpret_cast<T*>(old_front);
    }

    /** Deallocate an object returned by pop_front().
    *
    * This function must be used to destroy objects that are returned by
    * pop_front().
    * Do not use this function with an object that was not returned by
    * pop_front(), even if the type matches.
    *
    * @param obj A pointer to an object returned by pop_front().
    *
    * @note This function is Thread-safe, lock-free but not wait-free.
    */
    void deallocate(T* obj) noexcept
    {
        if (!obj) return;

        // call destructor
        NodeAllocatorTraits::destroy(alc_, reinterpret_cast<node<T>*>(obj));

        // nodes with next == 0 are still referenced by an executing
        // push_back() function and the next ptr will be modified.
        // Since we don't want push_back() to write to deallocated memory, we hang
        // in a loop until the node has a non-zero next ptr.
        while(!reinterpret_cast<node<T>*>(obj)->next.load())
            std::this_thread::yield();

        NodeAllocatorTraits::deallocate(
            alc_, reinterpret_cast<node<T>*>(obj), 1
        );
    }


    /** Get size of the queue.
    *
    * This function returns an approximation of the current queue size.
    * In a multithreaded environment, this value might change immediately
    * before or after the invocation. Especially, it is *not* guaranteed that
    * if(size() != 0) assert(pop_front() != nullptr) !
    *
    * In multithreaded environments, please consider to not use this function.
    *
    * @return Current size of the queue.
    *
    * @note This function is Thread-safe, lock-free and wait-free.
    */
    std::size_t size() const noexcept
    { return size_; }

protected:

    void push_node(node<T>* new_node) noexcept
    {
        node<T>* old_back = back_.exchange(new_node);
        node<T>* null_node = nullptr;

        // if front_ was set to null (no node yet), we have to update the front_
        // pointer.
        if(!front_.compare_exchange_strong(null_node, new_node))
            // if front_ is not null, then there was a previous node.
            // We have to update this nodes next pointer.
            old_back->next  = new_node;

        ++size_;
    }

    typedef Allocator ValueAllocator;
    typedef std::allocator_traits<ValueAllocator> ValueAllocatorTraits;

    // Rebind allocator traits for ValueAllocator to our own NodeAllocator
    typedef
        typename ValueAllocatorTraits::template rebind_traits<node<T> >
#if defined(_MSC_VER) && _MSC_VER <= 1700
		::other
#endif
        NodeAllocatorTraits;

    // Get actual allocator type from traits
    typedef typename NodeAllocatorTraits::allocator_type NodeAllocator;

    std::atomic_size_t size_ /**< Current size of queue. Not reliable. */;
    std::atomic<node<T>*> front_ /**< Front of the queue. */;
    std::atomic<node<T>*> back_ /**< Back of the queue. */;
    NodeAllocator alc_ /**< Allocator for node<T> objects. */;
};

} // namespace detail

using detail::atomic_queue_base;

} // namespace aq



#endif // ifndef ATOMIC_QUEUE_HPP_INCLUDED
