#ifndef DATAMERGER_H
#define DATAMERGER_H

// Header for containers
#include <queue>
#include <vector>
#include <functional>

// Headers for handling threads
#include <mutex>
#include <atomic>
#include <condition_variable>

#include "event_container.h"

#if TEMPLATE_IMPLEMENTATION_DMRG
#define TEMPLATE_TEXT_MRG template<class T=Event_t, class Compare=std::greater<T> >
#define TEMPLATE_CONTAINER_MRG std::vector<T>
#define TEMPLATE_QUEUE_MRG std::priority_queue<T, std::vector<T>, Compare>
#define TEMPLATE_CLASS_NAME_MRG EventMerger<T, Compare, Schedule>
#else
#define TEMPLATE_TEXT_MRG
#define TEMPLATE_CONTAINER_MRG std::vector<Event_t>
#define TEMPLATE_QUEUE_MRG std::priority_queue<Event_t, std::vector<Event_t>, std::greater<Event_t> >
#define TEMPLATE_CLASS_NAME_MRG EventMerger
#endif // TEMPLATE_IMPLEMENTATION


// Todo:
// This class should be event type indendent as the process of merging the data should be
// independent of the structure of the data. However this is not critical for our use since
// we will use a rather spesific implementation of the event structure that is not expected
// to change. In principle this clas won't need to know anything about the event type, only
// that there is a type called Event_t.


TEMPLATE_TEXT_MRG
class EventMerger {

public:

    // Constructor
    //EventMerger();

    // Destructor
    ~EventMerger(){}

    // Sorting thread. This thread is the main loop
    // of the thread that will perform the work.
    void SortThread();

    // Function that will signal the thread that it has
    // recived all the data that it will be reciving and
    // that it can terminate once it has finished sorting
    // all the data. This function will block the caller
    // until the SortThread has finished and terminated.
    // Return True once the thread has terminated.
    bool Terminate(int seconds_to_wait=20 /* Number of seconds that we will be waiting for the thread to die    */);

    // Check if the thread is running.
    bool IsRunning() const { return alive_thread; }

    // Add data
    void AddData(TEMPLATE_CONTAINER_MRG pEvents);

    // Get data in the order that it should be written
    // to the disk. This will return an empty
    TEMPLATE_CONTAINER_MRG GetData(const int &min_readout   /*!< We won't read out data unless we have more than this amount    */);

    // Force flush of all the data from the sorted_events queue.
    // This is useful when the run has ended and we are just saving
    // the last few buffers of data.
    TEMPLATE_CONTAINER_MRG FlushData();

private:

    // Container for the data that is added by AddData.
    TEMPLATE_CONTAINER_MRG recived_buffers;

    // Container for the sorted and merged events.
    TEMPLATE_QUEUE_MRG sorted_events;

    // Some logistics for the threads to be effective.
    // We want the sort thread to sleep while we are
    // waiting for more events.
    //Schedule *m_parent;

    // Mutex that we will lock whenever someone are writing or reading to recived_buffers.
    std::mutex rb_mutex;

    // Mutex that we will lock whenever someone are writing or reading to sorted_events.
    std::mutex se_mutex;

    // A mutex that we will use to hault the sorting loop while we are waiting for more data.
    std::mutex wait_mutex;

    // Conditional variable that we use to signal the sorter thread that it has work to do.
    std::condition_variable data_ready;

    // A mutex that we will use while waiting for the sort thread to die.
    std::mutex kill_mutex;

    // Bool flag to signal that the thread should be running.
    std::atomic_bool alive_thread;

    // Conditional variable that we use to signal the schedule thread that we have
    // indeed stopped the thread.
    std::condition_variable its_a_thread_lock_life_for_me;

};

#endif // DATAMERGER_H
