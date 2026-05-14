//
// Created by Vetle Wegner Ingeberg on 16/04/2021.
//

#ifndef TDR2TREE_TRIGGER_H
#define TDR2TREE_TRIGGER_H

#include <Tasks/Task.h>
#include <Tasks/Queue.h>
#include <Tasks/RingBuffer.h>

#include <Configuration/UserConfiguration.h>

namespace Task {

    class Trigger : public Base
    {
    private:
        MCEventQueue_t &input_queue;
        MTEventQueue_t output_queue;
        RingBuffer stats;
        const UserConfiguration& config;

    public:
        CLASS_NAME(Trigger)
        Trigger(MCEventQueue_t &input, const UserConfiguration& config);
        MTEventQueue_t &GetQueue(){ return output_queue; }

        RingBuffer &GetStats(){ return stats; }
        void Run() override;

    };
}

#endif //TDR2TREE_TRIGGER_H
