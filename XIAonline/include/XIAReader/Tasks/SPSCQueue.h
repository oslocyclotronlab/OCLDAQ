//
// Created by Vetle Wegner Ingeberg on 06/03/2026.
//

#ifndef SPSCQUEUE_H
#define SPSCQUEUE_H

#include <atomic>
#include <array>
#include <bit>
#include <cassert>
#include <chrono>
#include <thread>

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

    // timeout: how long to wait for an entry before failing when the queue is empty.
    // A value of zero (the default) waits indefinitely. std::atomic::wait() has no
    // timed overload, so a non-zero timeout is honored by polling in short slices
    // instead of blocking on tail_.wait(), which lets callers periodically re-check
    // a "done" flag instead of blocking forever.
    bool wait_and_pop(T& out, std::chrono::duration<double> timeout = std::chrono::duration<double>::zero()) {
        size_t head = head_local_;
        const bool has_timeout = timeout > std::chrono::duration<double>::zero();
        const auto deadline = has_timeout
            ? std::chrono::steady_clock::now() + std::chrono::duration_cast<std::chrono::steady_clock::duration>(timeout)
            : std::chrono::steady_clock::time_point{};

        while ( true ) {
            tail_cache_ = tail_.load(std::memory_order_acquire);
            if (head != tail_cache_) {
                out = std::move(buffer_[head & mask]);
                head_local_ = head + 1;
                head_.store(head_local_, std::memory_order_release);
                head_.notify_one();
                return true;
            }
            if (is_finish)
                return false;

            if (!has_timeout) {
                tail_.wait(tail_cache_);
            } else {
                const auto now = std::chrono::steady_clock::now();
                if (now >= deadline)
                    return false;

                constexpr std::chrono::steady_clock::duration poll_interval = std::chrono::milliseconds(10);
                std::this_thread::sleep_for(std::min(poll_interval, deadline - now));
            }
            head = head_local_;
        }
    }

    void mark_as_finish() {
        is_finish = true;
        tail_.notify_all();
    }
    bool is_not_finish() const { return !is_finish; }
    size_t size() const {
        return tail_.load(std::memory_order_acquire) - head_.load(std::memory_order_acquire);
    }
    constexpr size_t capacity() const { return Capacity; }
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