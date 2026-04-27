//
// Created by Vetle Wegner Ingeberg on 16/04/2021.
//

#ifndef SPLITTERSE_H
#define SPLITTERSE_H

#include <Tasks/Task.h>
#include <Tasks/Queue.h>

namespace Task {

    class Splitter : public Base
    {
    private:
        EntryQueue_t &input_queue;
        MCEventQueue_t output_queue;
        const double gap;
        //std::vector<word_t> buffer;

        void SplitEntries();

    public:
        CLASS_NAME(Splitter)
        Splitter(EntryQueue_t &input, const double &time_gap = 1500., const size_t &cap = 65536);
        MCEventQueue_t &GetQueue(){ return output_queue; }
        void Run() override;

    };

}

#endif //SPLITTERSE_H
