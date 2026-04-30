//
// Created by Vetle Wegner Ingeberg on 09/03/2022.
//

#ifndef XIAREADER_H
#define XIAREADER_H


#include "Task.h"
#include "Queue.h"

#include <Configuration/ConfigManager.h>

namespace Task {

    class Unpacker : public Base {
    public:
        CLASS_NAME(Unpacker)
        Unpacker(InputQueue_t &input_queue, ConfigManager config_manager);

        auto &GetQueue(){ return output_queue; }
        void Run() override;

    private:
        InputQueue_t &input_queue;
        EntryQueue_t output_queue;

        ConfigManager config;

        std::vector<uint32_t> data;
        std::vector<uint32_t> overflow;
    };

}

#endif // XIAREADER_H
