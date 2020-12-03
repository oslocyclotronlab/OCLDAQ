//
// Created by Vetle Wegner Ingeberg on 03/12/2020.
//

#ifndef OCLDAQ_PUBLISHER_H
#define OCLDAQ_PUBLISHER_H

#include "Event.h"

#include "zmq.hpp"
#include "readerwriterqueue.h"


#include <thread>
#include <vector>
#include <string>

using queue_t = moodycamel::BlockingReaderWriterQueue<word_t>;

/*!
 * publisher class
 * A simple class that takes waits for buffers from the unpacker and publishes them on the network.
 */
class publisher
{

private:

    zmq::context_t context;
    zmq::socket_t publish_socket;

    queue_t queue;

    bool stop;
    std::thread worker;

    //void worker_thread(std::stop_token token);

public:

    explicit publisher(const std::string& addr);

    ~publisher();

    void AddBuffer(const std::vector<word_t> &buffer);
};


#endif //OCLDAQ_PUBLISHER_H
