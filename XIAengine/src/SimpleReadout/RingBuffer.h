//
// Created by Vetle Wegner Ingeberg on 08/06/2026.
//

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

template<typename T>
struct ContiguousBlock
{
    const T* data = nullptr;
    std::size_t size = 0;

    const T& operator[](std::size_t i) const
    {
        return data[i];
    }
};

template<typename T>
class RingBuffer
{
public:
    explicit RingBuffer(std::size_t capacity)
        : capacity_(capacity)
        , storage_(capacity * 2)
    {
        if (capacity == 0) {
            throw std::invalid_argument("RingBuffer capacity must be > 0");
        }
    }

    [[nodiscard]] bool empty() const
    {
        return size_ == 0;
    }

    [[nodiscard]] bool full() const
    {
        return size_ == capacity_;
    }

    [[nodiscard]] std::size_t size() const
    {
        return size_;
    }

    [[nodiscard]] std::size_t capacity() const
    {
        return capacity_;
    }

    [[nodiscard]] std::size_t available() const noexcept
    {
        return capacity_ - size_;
    }

    void clear()
    {
        head_ = 0;
        tail_ = 0;
        size_ = 0;
    }

    void push(const T& value)
    {
        write_at(tail_, value);

        if (full()) {
            head_ = increment(head_);
        } else {
            ++size_;
        }

        tail_ = increment(tail_);
    }

    void push(T&& value)
    {
        write_at(tail_, std::move(value));

        if (full()) {
            head_ = increment(head_);
        } else {
            ++size_;
        }

        tail_ = increment(tail_);
    }

    template<typename InputIt>
    void push(InputIt first, InputIt last)
    {
        for (; first != last; ++first) {
            push(*first);
        }
    }

    void pop()
    {
        if (empty()) {
            throw std::out_of_range("RingBuffer is empty");
        }

        head_ = increment(head_);
        --size_;
    }

    void consume(std::size_t count)
    {
        count = std::min(count, size_);

        head_ = (head_ + count) % capacity_;
        size_ -= count;
    }

    [[nodiscard]] T& front()
    {
        if (empty()) {
            throw std::out_of_range("RingBuffer is empty");
        }

        return storage_[head_];
    }

    [[nodiscard]] const T& front() const
    {
        if (empty()) {
            throw std::out_of_range("RingBuffer is empty");
        }

        return storage_[head_];
    }

    [[nodiscard]] T& back()
    {
        if (empty()) {
            throw std::out_of_range("RingBuffer is empty");
        }

        return storage_[tail_ == 0 ? capacity_ - 1 : tail_ - 1];
    }

    [[nodiscard]] const T& back() const
    {
        if (empty()) {
            throw std::out_of_range("RingBuffer is empty");
        }

        return storage_[tail_ == 0 ? capacity_ - 1 : tail_ - 1];
    }

    /// Pointer to the first element.
    /// Because of the mirrored storage, the next size() elements
    /// are contiguous in memory.
    [[nodiscard]] T* data()
    {
        return storage_.data() + head_;
    }

    [[nodiscard]] const T* data() const
    {
        return storage_.data() + head_;
    }

    /// Convenience for XIA parsing.
    [[nodiscard]] const T* front_ptr() const
    {
        return data();
    }

    /// Peek at N contiguous elements.
    [[nodiscard]] const T* peek(std::size_t count) const
    {
        if (count > size_) {
            throw std::out_of_range("Not enough elements");
        }

        return data();
    }

    ContiguousBlock<T> pop_contiguous(std::size_t n)
    {
        if (n > size_) {
            throw std::out_of_range("Not enough data in ring buffer");
        }

        // Because of mirrored storage:
        // storage_[head_ ... head_ + capacity_ - 1] is always contiguous
        return ContiguousBlock<T>{
            storage_.data() + head_,
            n
        };
    }

    void commit(std::size_t n)
    {
        if (n > size_) {
            throw std::out_of_range("Commit exceeds buffer size");
        }

        head_ += n;
        if (head_ >= capacity_) {
            head_ -= capacity_;
        }

        size_ -= n;
    }

private:
    template<typename U>
    void write_at(std::size_t index, U&& value)
    {
        storage_[index] = std::forward<U>(value);
        storage_[index + capacity_] = storage_[index];
    }

    [[nodiscard]] std::size_t increment(std::size_t idx) const
    {
        ++idx;
        if (idx == capacity_) {
            idx = 0;
        }
        return idx;
    }

private:
    std::size_t capacity_;
    std::vector<T> storage_;

    std::size_t head_ = 0;
    std::size_t tail_ = 0;
    std::size_t size_ = 0;
};

#endif // RINGBUFFER_H
