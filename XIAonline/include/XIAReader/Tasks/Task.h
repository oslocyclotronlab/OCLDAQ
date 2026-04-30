//
// Created by Vetle Wegner Ingeberg on 16/04/2021.
//

#ifndef TDR2TREE_TASK_H
#define TDR2TREE_TASK_H

#include <atomic>
#include <utility>
#include <thread>
#include <vector>
#include <optional>
#include <iostream>

#ifndef CLASS_NAME
#define CLASS_NAME(class_name) \
    const char* name() override { return #class_name; }
#endif // CLASS_NAME

namespace TDR {
    struct Entry_t;
}

//struct word_t;

namespace Task {

    class Base
    {
    protected:
        std::atomic<bool> is_done = false;
        //std::stop_token done;
        std::atomic<bool> done = false;
        std::optional<std::exception> exception;
        size_t entries_processed = 0;
    public:
        virtual const char* name() = 0;
        virtual ~Base() = default;

        void Finish() { done = true; }
        virtual void Run() = 0;
        size_t GetEntriesProcessed() const { return entries_processed; }

        // Check if an exception was thrown
        bool check_status(){
            if ( exception.has_value() ){
                std::cout << "I'm throwing exception: " << exception.value().what() << std::endl;
                throw std::exception(exception.value());
            }
            return is_done;
        }

        inline void check_exception()
        {
            if ( exception.has_value() ){
                throw std::exception(exception.value());
            }
        }

        // Check the status, but do not throw
        inline bool check_status_nothrow() noexcept { return bool( is_done ); }

        void DoRun()
        {
            try {
                this->Run();
            } catch ( const std::exception &ex ){
                std::cerr << "Task " << typeid(*this).name() << " got exception '" << ex.what() << "'" << std::endl;
                exception = ex;
            }
            is_done = true;
        }

        std::pair<std::thread, Base *> ConstructThread()
        {
            return std::make_pair(std::thread(&Base::Run, this), this);
        }

    };
}

#endif //TDR2TREE_TASK_H
