#include <atomic>
#include <array>
#include <optional>
#include <thread>
#include <cstddef>

template<typename T, size_t Capacity>
class SPMCBlockingQueue
{
    static_assert((Capacity & (Capacity - 1)) == 0,
                  "Capacity must be power of two");

private:

    static constexpr size_t MASK = Capacity - 1;

    alignas(64) std::array<T, Capacity> buffer;

    alignas(64) std::atomic<size_t> read_index{0};
    alignas(64) std::atomic<size_t> write_index{0};

    bool is_finish = false;

public:

    bool empty() const
    {
        return write_index.load(std::memory_order_acquire) ==
               read_index.load(std::memory_order_acquire);
    }

    // Single producer (blocking when full)
    void push(T value)
    {
        size_t w;

        while (true)
        {
            w = write_index.load(std::memory_order_relaxed);
            size_t r = read_index.load(std::memory_order_acquire);

            if (w - r < Capacity)
                break;

            std::this_thread::yield();  // queue full
        }

        buffer[w & MASK] = std::move(value);

        write_index.store(w + 1, std::memory_order_release);
    }

    // Multiple consumers
    std::optional<T> pop()
    {
        while (true)
        {
            size_t r = read_index.load(std::memory_order_relaxed);
            size_t w = write_index.load(std::memory_order_acquire);

            if (r == w)
                return std::nullopt;

            if (read_index.compare_exchange_weak(
                    r,
                    r + 1,
                    std::memory_order_acq_rel,
                    std::memory_order_relaxed))
            {
                T value = std::move(buffer[r & MASK]);
                return value;
            }
        }
    }

    void mark_as_finish(){ is_finish = true; }
    bool is_not_finish() const { return !is_finish; }
};