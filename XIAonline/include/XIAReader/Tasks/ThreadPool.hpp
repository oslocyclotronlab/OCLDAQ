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
        // Stop the tasks in pipeline order (the order they were added): stop
        // one task and wait for it to exit before stopping the next. A task
        // may still push to its output queue while it shuts down (Buffer
        // flushes its whole sort buffer), and push() blocks while the queue is
        // full. If the consumer was already stopped, nobody would empty the
        // queue and the join would hang forever. Stopping in order keeps every
        // consumer running until its producer has exited.
        for ( auto &thread : tasks ) {
            thread.second->Finish();
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
