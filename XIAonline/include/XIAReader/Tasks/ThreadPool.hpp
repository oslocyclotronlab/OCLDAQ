//
// Created by Vetle Wegner Ingeberg on 05/10/2022.
//

#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include <utility>
#include <vector>

template<typename T>
class ThreadPool
{
private:
    using task_pair = std::pair<T, Task::Base *>;
    std::vector<task_pair> tasks;
    task_pair *source;

    void end_all()
    {
        for ( auto &thread : tasks ) {
            thread.second->Finish();
        }

        // Then we will try to join all the threads.
        for ( auto &thread : tasks ) {
            if ( thread.first.joinable() )
                thread.first.join();
        }

    }

public:
    ThreadPool()
    {
        tasks.reserve(256);
    }

    void AddTask(Task::Base *task)
    {
        tasks.push_back(task->ConstructThread());
        if ( tasks.size() == 1 )
            source = tasks.data();
    }

    void Wait()
    {

        // Wait for the producer thread to finish.
        if ( source->first.joinable() )
            source->first.join();

        // Check for error
        try {
            source->second->check_exception();
        } catch ( const std::exception &ex ){
            std::cerr << "Producer got exception '" << ex.what() << "'" << std::endl;
        }

        // Iterate over each thread, notifying them that they don't need to wait anymore.
        for ( auto &thread : tasks ){
            thread.second->Finish();
            if ( thread.first.joinable() )
                thread.first.join();
            try {
                source->second->check_exception();
            } catch ( const std::exception &ex ){
                std::cerr << "Got exception '" << ex.what() << "'" << std::endl;
            }
        }
    }

    void DoEnd()
    {
        end_all();
        try {
            for ( auto &thread : tasks )
                thread.second->check_status();
        } catch (std::exception &ex) {
            std::cerr << "Got an exception. Aborting." << std::endl;

        }
    }

    std::map<std::string, size_t> GetProcessedData() const {
        std::map<std::string, size_t> m;
        for (auto& task : tasks) {
                m[task.second->name()] = task.second->GetEntriesProcessed();
        }
        return m;
    }

};

#endif //XIA2TREE_THREADPOOL_HPP
