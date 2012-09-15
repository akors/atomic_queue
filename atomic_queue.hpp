#ifndef ATOMIC_QUEUE_HPP_INCLUDED
#define ATOMIC_QUEUE_HPP_INCLUDED

#include <cstddef>
#include <memory>
#include <atomic>
#include <thread>

namespace aq {

namespace detail {

template <typename T>
struct node
{
    T t;
    std::atomic<node<T>*> next;
};


template <typename T, typename Allocator = std::allocator<T> >
class atomic_queue
{
public:
    atomic_queue(const Allocator& alc = Allocator())
        : size_(0u), front_(nullptr), end_(nullptr),
        alc_(alc)
    {
    }

    // NOT threadsafe!!! Why would you call the destructor if the object is in
    // use? Are you crazy?
    ~atomic_queue()
    {
        node<T>* fr = front_; // atomic_load

        while(fr)
        {
            node<T>* next = fr->next;
            alc_.destroy(fr);
            alc_.deallocate(fr, 1);
            fr = next;
        }

    }

    void push(const T& t)
    {
        typename NodeAllocator::pointer new_node = alc_.allocate(1);

        try {
            ValueAllocator(alc_).construct(&new_node->t, t);
        } catch(...)
        {
            alc_.deallocate(new_node, 1);
            throw;
        }
        new_node->next = nullptr;

        push_node(new_node);
    }

    T* pop()
    {
        node<T>* old_front = front_;
        node<T>* new_front;

        do {
            if (!old_front) return nullptr; // nothing to pop
            new_front = old_front->next;
        } while(!front_.compare_exchange_weak(old_front, new_front));

        --size_;

        // if the old front was also the end, the queue is now empty.
        new_front = old_front;
        if(end_.compare_exchange_strong(new_front, nullptr))
            old_front->next = old_front;

        return reinterpret_cast<T*>(old_front);
    }

    void deallocate(T* obj)
    {
        if (!obj) return;

        // call destructor
        alc_.destroy(reinterpret_cast<node<T>*>(obj));

        // nodes with next == 0 are still referenced by an executing
        // push() function and the next ptr will be modified.
        // Since we don't want the function to write to deallocated
        // memory, we hang in a loop until the node has a non-zero next ptr.
        while(!reinterpret_cast<node<T>*>(obj)->next.load())
            std::this_thread::yield();

        alc_.deallocate(reinterpret_cast<node<T>*>(obj), 1);
    }

    std::size_t size() const
    {
        return size_;
    }
protected:

    void push_node(node<T>* new_node)
    {
        node<T>* old_end = end_.exchange(new_node);
        node<T>* null_node = nullptr;

        // if front_ was set to null (no node yet), we have to update the front_
        // pointer.
        if(!front_.compare_exchange_strong(null_node, new_node))
            // if front_ is not null, then there was a previous node.
            // We have to update this nodes next pointer.
            old_end->next  = new_node;

        ++size_;
    }

    typedef Allocator ValueAllocator;
    typedef typename Allocator::template rebind<node<T> >::other NodeAllocator;

    std::atomic_size_t size_;
    std::atomic<node<T>*> front_;
    std::atomic<node<T>*> end_;
    NodeAllocator alc_;
};

} // namespace detail

using detail::atomic_queue;

} // namespace aq



#endif // ifndef ATOMIC_QUEUE_HPP_INCLUDED
