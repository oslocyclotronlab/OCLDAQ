//
// Created by Vetle Wegner Ingeberg on 06/03/2026.
//

#ifndef SPSCQUEUE_H
#define SPSCQUEUE_H

#include <atomic>
#include <array>
#include <bit>
#include <cassert>

template<typename T, size_t Capacity>
class SPSCBlockingQueue
{
    static_assert(std::has_single_bit(Capacity),
        "Capacity must be power of two");

public:
    void push(T value)
    {
        size_t tail = tail_local_;

        if (tail - head_cache_ == Capacity) {
            head_cache_ = head_.load(std::memory_order_acquire);

            while (tail - head_cache_ == Capacity) {
                head_.wait(head_cache_);
                head_cache_ = head_.load(std::memory_order_acquire);
            }
        }

        buffer_[tail & mask] = std::move(value);

        tail_local_ = tail + 1;
        tail_.store(tail_local_, std::memory_order_release);
        tail_.notify_one();
    }

    T pop()
    {
        size_t head = head_local_;

        if (head == tail_cache_) {
            tail_cache_ = tail_.load(std::memory_order_acquire);

            while (head == tail_cache_) {
                tail_.wait(tail_cache_);
                tail_cache_ = tail_.load(std::memory_order_acquire);
            }
        }

        T value = std::move(buffer_[head & mask]);

        head_local_ = head + 1;
        head_.store(head_local_, std::memory_order_release);
        head_.notify_one();

        return value;
    }

    bool try_pop(T& out)
    {
        size_t head = head_local_;

        if (head == tail_cache_) {
            tail_cache_ = tail_.load(std::memory_order_acquire);

            if (head == tail_cache_)
                return false;
        }

        out = std::move(buffer_[head & mask]);

        head_local_ = head + 1;
        head_.store(head_local_, std::memory_order_release);
        head_.notify_one();

        return true;
    }

    void mark_as_finish(){ is_finish = true; }
    bool is_not_finish() const { return !is_finish; }
    bool empty() {
        size_t head = head_local_;

        if (head != tail_cache_)
            return false;

        tail_cache_ = tail_.load(std::memory_order_acquire);

        return head == tail_cache_;
    }

private:

    static constexpr size_t mask = Capacity - 1;

    alignas(64) std::atomic<size_t> head_{0};
    alignas(64) std::atomic<size_t> tail_{0};

    alignas(64) std::array<T, Capacity> buffer_;

    // producer-local
    size_t tail_local_ = 0;
    size_t head_cache_ = 0;

    // consumer-local
    size_t head_local_ = 0;
    size_t tail_cache_ = 0;

    std::atomic_bool is_finish = false;
};

#endif // SPSCQUEUE_H