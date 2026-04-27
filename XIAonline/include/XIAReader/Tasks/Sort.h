//
// Created by Vetle Wegner Ingeberg on 06/03/2026.
//

#ifndef SORT_H
#define SORT_H




#include <Tasks/Task.h>
#include <Tasks/Queue.h>

#include <Configuration/UserConfiguration.h>

namespace Task {

    class Sorter  : public Base
    {
    private:
        TEventQueue_t &input_queue;
        const UserConfiguration& configuration;

    public:
        CLASS_NAME(Sorter)
        Sorter(TEventQueue_t &input, const UserConfiguration& config);
        ~Sorter() override = default;
        void Run() override;
    };
};

#endif // SORT_H