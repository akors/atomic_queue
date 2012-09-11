
#ifndef ATOMIC_QUEUE_HPP_INCLUDED
#define ATOMIC_QUEUE_HPP_INCLUDED

#include <cstddef>
#include <memory>

namespace aq {

namespace detail {

template <typename T>
struct node
{
    T t;
    node<T>* next;
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


    void push(const T& t)
    {
        typename NodeAllocator::pointer new_obj = alc_.allocate(1);
        ValueAllocator(alc_).construct(&new_obj->t, t);
        new_obj->next = nullptr;

        // old_end = end.atomic_exchange(new_obj);
        node<T>* old_end = end_; end_ = new_obj;

        // if we had a previous node,
        if (old_end) // update next pointer
            old_end->next = new_obj; // maybe atomic too?
        else // if we didn't, we probably need to set the front pointer
        {
            // atomic_compare_exchange(front_, &old_end, new_obj)
            if(front_ == old_end) front_ = new_obj;
        }

        ++size_;
    }

    void deallocate(T* obj)
    {
        if (obj)
        alc_.deallocate(reinterpret_cast<typename NodeAllocator::pointer>(obj));
    }

    std::size_t size() const
    {
        return size_;
    }
private:
    typedef Allocator ValueAllocator;
    typedef typename Allocator::template rebind<node<T>>::other NodeAllocator;

    NodeAllocator alc_;
    std::size_t size_;

    node<T>* front_;
    node<T>* end_;
};

} // namespace detail

using detail::atomic_queue;

} // namespace aq



#endif // ifndef ATOMIC_QUEUE_HPP_INCLUDED
