#include "DataMerger.h"

#include <chrono>
#include <iostream>

#define MIN_SORT 256
#define MAX_WAIT 1 /* s */

/*********** Local variables only used in our implementation *************/
bool you_can_sort_data = false;
bool thread_is_dead = true;

bool CanISort()
{
    return you_can_sort_data;
}

bool IsItDeadYet()
{
    return thread_is_dead;
}

/*********** Local variables only used in our implementation *************/

TEMPLATE_TEXT_MRG
void TEMPLATE_CLASS_NAME_MRG::SortThread()
{
    // We make sure that the there is a flag indicating that we are currently running.
    alive_thread = true;
    thread_is_dead = false;


    // Start of thread loop
    do {

        // First we check if we have any data. Here we are blocking the thread.
        // We will however check once a second to be sure though.
        { // Start of scope
            std::unique_lock<std::mutex> lk(wait_mutex);
            data_ready.wait_for(lk, std::chrono::milliseconds(MAX_WAIT), [](){ return you_can_sort_data; });
            lk.unlock();
        } // End of scope


        { // New scope
            std::lock_guard<std::mutex> rb_guard(rb_mutex);

            if (recived_buffers.size() > MIN_SORT){
                std::lock_guard<std::mutex> se_guard(se_mutex);

                for (size_t i = 0 ; i < recived_buffers.size() ; ++i){
                    sorted_events.push(recived_buffers[i]);
                }
                recived_buffers.clear();
            }

            // We want to stop at the lock again!
            you_can_sort_data = false;
        } // EOS

    } while (alive_thread);

    // Once we are finished running the thread we will do one last check to make sure
    // we have sorted all the data. In principle, after this pice of code has ran,
    // we won't be able to sort any of the incomming data.

    // In principle, at this point in time there shouldn't be anyone else moving data
    // around at this point, except for the file writer. The file writer will be writing
    // to the
    std::lock_guard<std::mutex> rb_guard(rb_mutex);
    if (recived_buffers.size() > 0){
        std::lock_guard<std::mutex> se_guard(se_mutex);

        for (size_t i = 0 ; i < recived_buffers.size() ; ++i){
            sorted_events.push(recived_buffers[i]);
        }
        recived_buffers.clear();
    }

    thread_is_dead = true; // We will notify any interested parties that we ended.

    return; // Ladies and gentlemen, we have terminated!
}

TEMPLATE_TEXT_MRG
bool TEMPLATE_CLASS_NAME_MRG::Terminate(int seconds_to_wait)
{
    alive_thread = false;
    std::unique_lock<std::mutex> kill_lock(kill_mutex);
    its_a_thread_lock_life_for_me.wait_for(kill_lock, std::chrono::seconds(seconds_to_wait), [](){ return thread_is_dead; });

    kill_lock.unlock();
    return thread_is_dead; // If we return False, please inform the user that a crash might be imminent.
}

TEMPLATE_TEXT_MRG
void TEMPLATE_CLASS_NAME_MRG::AddData(TEMPLATE_CONTAINER_MRG pEvents)
{
    std::lock_guard<std::mutex> rb_guard(rb_mutex);

    // Append to the current recived buffers
    for (size_t i = 0 ; i < pEvents.size() ; ++i){
        recived_buffers.push_back(pEvents[i]);
    }
    you_can_sort_data = true;
    data_ready.notify_all();
}

TEMPLATE_TEXT_MRG
TEMPLATE_CONTAINER_MRG TEMPLATE_CLASS_NAME_MRG::GetData(const int &min_readout)
{
    TEMPLATE_CONTAINER_MRG rData;

    std::lock_guard<std::mutex> se_lock(se_mutex);

    if (sorted_events.size() > MIN_SORT + min_readout){
        while (sorted_events.size() >= MIN_SORT){
            rData.push_back(sorted_events.top());
            sorted_events.pop();
        }
    }

    return rData;
}

TEMPLATE_TEXT_MRG
TEMPLATE_CONTAINER_MRG TEMPLATE_CLASS_NAME_MRG::FlushData()
{
    TEMPLATE_CONTAINER_MRG rData;

    std::lock_guard<std::mutex> se_lock(se_mutex);

    while (!sorted_events.empty()){
        rData.push_back(sorted_events.top());
        sorted_events.pop();
    }
    return rData;
}
