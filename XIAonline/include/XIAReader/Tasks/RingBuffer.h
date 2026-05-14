//
// Created by Vetle Wegner Ingeberg on 30/04/2026.
//

#ifndef BUILDALL_RINGBUFFER_H
#define BUILDALL_RINGBUFFER_H

#include <vector>
#include <mutex>

class RingBuffer {
public:
    explicit RingBuffer(size_t size)
        : buffer(size, 0.0),
          maxSize(size),
          index(0),
          count(0),
          sum(0.0) {}

    void Add(double value) {
        std::lock_guard<std::mutex> lock(mtx);

        sum -= buffer[index];
        buffer[index] = value;
        sum += value;

        index = (index + 1) % maxSize;

        if (count < maxSize) {
            ++count;
        }
    }

    double GetAvg() const {
        std::lock_guard<std::mutex> lock(mtx);

        if (count == 0) return 0.0;
        return sum / count;
    }

    double GetSum() const {
        std::lock_guard<std::mutex> lock(mtx);
        return sum;
    }

private:
    std::vector<double> buffer;
    size_t maxSize;
    size_t index;
    size_t count;
    double sum;

    mutable std::mutex mtx;
};

#endif //BUILDALL_RINGBUFFER_H
