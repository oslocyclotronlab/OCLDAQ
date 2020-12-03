//
// Created by Vetle Wegner Ingeberg on 03/12/2020.
//

#include "publisher.h"

#define HAVE_WORD_T
#include "serializer.hpp"

#include <iostream>

#define MAX_BUFFER_SIZE 131072
#define INTERNAL_BUFFER_SIZE 65536

void worker_thread(const std::stop_token &token, queue_t &queue, zmq::socket_t &publish_socket)
{
    std::vector<word_t> words(INTERNAL_BUFFER_SIZE);
    word_t word;
    int num = 0;
    while ( !token.stop_requested() ){
        // We will first fill 64k of entries in a vector, that vector has to be serialized.
        while ( words.size() < INTERNAL_BUFFER_SIZE ){
            if ( queue.wait_dequeue_timed(word, std::chrono::seconds(1)) ) {
                words.push_back(word);
            } else {
                std::cout << "Time-out, have " << words.size() << " entries" << std::endl;
                break;
            }
        }

        if ( words.size() >= INTERNAL_BUFFER_SIZE ){
            // Now we can transfer a single buffer <3
            publish_socket.send(zmq::buffer(to_string(words)));
            words.clear();
            std::cout << "Sending buffer# " << num++ << std::endl;
        }
    }

    if ( !words.empty() ){
        publish_socket.send(zmq::buffer(to_string(words)));
    }
}

publisher::publisher(const std::string& addr)
    : context()
    , publish_socket(context, zmq::socket_type::pub)
    , queue( MAX_BUFFER_SIZE )
{
    try {
        publish_socket.bind(addr);
    } catch (zmq::error_t &error) {
        std::cerr << "Could not bind to address '" << addr;
        std::cerr << "', got error '" << error.what() << "'." << std::endl;
        exit(EXIT_FAILURE);
    }

    // Start worker thread
    worker = std::jthread([&](const std::stop_token& token){ worker_thread(token, queue, publish_socket); });
}

void publisher::AddBuffer(const std::vector<word_t> &buffer)
{
    for ( auto &entry : buffer ){
        queue.enqueue(entry);
    }
}

publisher::~publisher()
{
    worker.request_stop();
    if ( worker.joinable() ){
        worker.join();
    }
}
