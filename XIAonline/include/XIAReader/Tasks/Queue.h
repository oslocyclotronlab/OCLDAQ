//
// Created by Vetle Wegner Ingeberg on 06/04/2022.
//

#ifndef QUEUE_H
#define QUEUE_H


#include <Format/entry.h>
#include <Format/event.h>
#include <Format/xiaformat.h>

#include <Tasks/SPSCQueue.h>
#include <Tasks/SPMCQueue.h>


#define SIZE 16384

namespace Task {

    template<typename T>
    class QueueWorker {
    private:
        T& _queue;
    public:
        QueueWorker(T& queue) : _queue(queue) {}
        ~QueueWorker() { _queue.mark_as_finish(); }
    };
    using InputQueue_t = SPSCBlockingQueue<std::vector<uint32_t>, SIZE>;
    using XIAQueue_t = SPSCBlockingQueue<XIA_base_t, SIZE>;
    using EntryQueue_t = SPSCBlockingQueue<Entry_t, SIZE>;
    using EventQueue_t = SPSCBlockingQueue<std::vector<Entry_t>, SIZE>;
    using MCEventQueue_t = SPSCBlockingQueue<std::vector<Entry_t>, SIZE>;
    using TEventQueue_t = SPSCBlockingQueue<std::pair<std::vector<Entry_t>, int>, SIZE>;
    using MTEventQueue_t = SPSCBlockingQueue<std::pair<std::vector<Entry_t>, int>, SIZE>;
}

#endif // QUEUE_H
