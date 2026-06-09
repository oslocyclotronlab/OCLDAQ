//
// Created by Vetle Wegner Ingeberg on 05/06/2026.
//

#include <deque>
#include <vector>
#include <span>
#include "../engine/xiaformat.h"

#include <chrono>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <thread>
#include <vector>

#include "RingBuffer.h"
#include "functions.h"
#include "WriteTerminal.h"
#include "XIAControl.h"
#include "pixie16app_export.h"

class ModuleHandler {
public:
    ModuleHandler(const int& module, const size_t& _min_readout = 16384)
        : module_id(module)
        , min_readout( _min_readout )
        , buffer( 262144 * 4 )
        , tmp( new uint32_t[262144] )
    {}

    [[nodiscard]] const XIA_base_t* front() const {
        return (buffer.size() >= 4) ? reinterpret_cast<const XIA_base_t *>(buffer.front_ptr()) : nullptr;
    }

    [[nodiscard]] const uint32_t* head() const { return buffer.front_ptr(); }

    ContiguousBlock<uint32_t> pop_contiguous(std::size_t n) {
        return buffer.pop_contiguous(n);
    }

    void commit(std::size_t n) {
        buffer.commit(n);
    }

    [[nodiscard]] int64_t timestamp() const {
        return front()->timestamp();
    }

    void ReadModule() {
        unsigned int fifo_length = 0;
        unsigned readout_size = 0;
        int ret = Pixie16CheckExternalFIFOStatus(&fifo_length, module_id);
        if (ret < 0) {
            throw std::runtime_error("Pixie16CheckExternalFIFOStatus failed, retval: " + std::to_string(ret));
        }

        if ( fifo_length < 1024 )
            return;

        readout_size =  fifo_length < buffer.available() ? fifo_length : buffer.available();
        readout_size = ( readout_size > 262144 ) ? 262144 : readout_size;
        ret = Pixie16ReadDataFromExternalFIFO(tmp.get(), readout_size, module_id);
        if (ret < 0) {
            throw std::runtime_error("Pixie16ReadDataFromExternalFIFO failed, retval: " + std::to_string(ret));
        }

        buffer.push(tmp.get(), tmp.get() + readout_size);
    }

    [[nodiscard]] bool ReadoutReady() const { return (buffer.size() > min_readout); }

private:
    const int module_id;
    const size_t min_readout;
    RingBuffer<uint32_t> buffer;
    std::unique_ptr<uint32_t[]> tmp;
};

class RunManager {
private:
    std::vector<ModuleHandler> modules;
    std::vector<ModuleHandler *> module_front;
    RingBuffer<uint32_t> fifo;

public:
    RunManager(int NumModules) : fifo( 16384 * 4 * NumModules ) {
        for (int i = 0; i < NumModules; ++i) {
            modules.emplace_back(i);
            module_front.push_back(&modules[i]);
        }
    }

    ContiguousBlock<uint32_t> pop_contiguous(std::size_t n) {
        return fifo.pop_contiguous(n);
    }

    void commit(std::size_t n) {
        fifo.commit(n);
    }

    [[nodiscard]] const uint32_t* head() const { return fifo.front_ptr(); }

    [[nodiscard]] size_t data_available() const { return fifo.size(); }

    void ReadModules() {
        bool ready = true;
        for ( auto &module : modules ) {
            module.ReadModule();
            ready = ( ready && module.ReadoutReady() );
        }

        if ( !ready ) {
            return;
        }

        // We only readout modules if all are ready for readout... maybe something else later
        while ( ready ) {
            std::sort(module_front.begin(), module_front.end(),
                [](ModuleHandler* lhs, ModuleHandler* rhs) {
                    return lhs->timestamp() < rhs->timestamp();
                });
            ModuleHandler *module_ptr = module_front.front();
            fifo.push(module_ptr->head(), module_ptr->head() + module_ptr->front()->eventLen);
            for ( auto &module : modules ) { ready = ( ready && module.ReadoutReady() ); }
        }
    }

};


int main() {

    WriteTerminal termWrite;

    // Get mapping
    unsigned short PXIMapping[PRESET_MAX_MODULES];
    unsigned short NumModules;
    for (unsigned short & mapping : PXIMapping)
        mapping = 0;

    try {
        auto mapping = ReadSlotMap();
        if ( mapping.size() >= PRESET_MAX_MODULES ){
            std::string errmsg = "Too many PCI devices found, found " + std::to_string(mapping.size());
            throw std::runtime_error(errmsg);
        }
        int set = 0;
        for ( auto &entry : mapping ){
            PXIMapping[set++] = entry;
        }
        NumModules = set;
    } catch ( std::exception &ex ){
        std::cerr << "Could not determine PLX slot mapping, got error " << ex.what() << std::endl;
        std::cerr << "Try with manual PXI slot mapping, e.g.:" << std::endl;
        exit(EXIT_FAILURE);
    }


    auto xiacontr = new XIAControl(&termWrite, PXIMapping);

    // We will now boot before anything else will happend.
    if ( !xiacontr->XIA_boot_all(false) )
        return 1;
    RunManager manager(NumModules);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto begin_time = std::chrono::high_resolution_clock::now();
    xiacontr->XIA_start_run();

    FILE* file = fopen("test.bin", "wb");

    while ( std::chrono::high_resolution_clock::now() - begin_time < std::chrono::seconds(10) ) {
        manager.ReadModules();
        if ( manager.data_available() > 1024 ) {
            if ( fwrite(manager.head(), sizeof(uint32_t), manager.data_available(), file) != manager.data_available() ) {
                std::cerr << "fwrite failed" << std::endl;
                break;
            }
            manager.commit(manager.data_available());
        }
    }
    xiacontr->XIA_end_run();
    for ( int i = 0 ; i < NumModules ; ++i ) {
        auto ret = Pixie16ExitSystem(i);
        if ( ret < 0 ) {
            std::cerr << "Unable to release resources" << std::endl;
        }
    }
    return 0;
}
