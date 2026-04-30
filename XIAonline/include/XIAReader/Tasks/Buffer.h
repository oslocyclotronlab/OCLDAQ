//
// Created by Vetle Wegner Ingeberg on 16/04/2021.
//

#ifndef BUFFERSE_H
#define BUFFERSE_H

#include <Tasks/Task.h>
#include <Tasks/Queue.h>

#include <queue>


namespace Task {

    class Buffer : public Base
    {
    private:
        struct Compare {
            bool operator()(const Entry_t &a, const Entry_t &b) const { return ( double(a.timestamp - b.timestamp) + (a.cfdcorr - b.cfdcorr) > 0); }
        };

        EntryQueue_t &input_queue;
        EntryQueue_t output_queue;

        const size_t size;
        std::priority_queue<Entry_t, std::vector<Entry_t>, Compare> buffer;

    public:
        CLASS_NAME(Buffer)
        Buffer(EntryQueue_t &input, const size_t &buf_size = 65536);
        EntryQueue_t &GetQueue(){ return output_queue; }
        void Run() override;

    };

}

#endif //BUFFERSE_H