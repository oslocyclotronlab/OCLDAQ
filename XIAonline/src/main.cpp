
#include <iostream>
#include <fstream>

#include <csignal>
#include <unistd.h>
#include <cstdlib>

#include <sys/time.h>
#include <sys/stat.h>

#include "engine_shm.h"
#include "net_control.h"
#include "utilities.h"

#include <histogram/SharedHistograms.h>
#include <histogram/RootWriter.h>
#include <logfault/logfault.h>

#include <XIAReader/Tasks/Unpacker.h>
#include <XIAReader/Tasks/Buffer.h>
#include <XIAReader/Tasks/Splitter.h>
#include <XIAReader/Tasks/Trigger.h>
#include <XIAReader/Tasks/SortSingles.h>
#include <XIAReader/Tasks/SortCoincidence.h>
#include <XIAReader/Tasks/ThreadPool.hpp>

#include <Configuration/UserConfiguration.h>
#include <sys/mman.h>

char leaveprog='n';
static int buffer_count=0,bad_buffer_count=0;

static line_server *ls_sort = nullptr;
static SharedHistograms *histograms = nullptr;

void keyb_int(int sig_num)
{
    if (sig_num == SIGINT) {
        printf("\n\nLeaving...\n");
        leaveprog = 'y';
    }
}

// ########################################################################

static void send_1_or_all(const std::string& message, line_channel* lc=0)
{
    if( lc )
        lc->send(message);
    else
        ls_sort->send_all(message);
}

// ########################################################################

static void command_quit(line_channel*, const std::string&, void*)
{
    leaveprog = 'y';
}


// ########################################################################

static void command_status_cwd(line_channel* lc, const std::string&, void*)
{
    char cwd[1024];
    if( !getcwd(cwd, sizeof(cwd)) ) {
        line_sender ls(lc);
        if( errno == ENOENT ) {
            ls << "207 status_cwd -unlinked-\n";
        } else {
            ls << "405 error_dir Cannot get current directory.\n";
        }
    } else {
        line_sender ls(lc);
        ls << "207 status_cwd " << cwd << '\n';
    }
}

// ########################################################################

static void command_change_cwd(line_channel* lc, const std::string& line, void*)
{
    const std::string dirname = line.substr(11);
    if( chdir(dirname.c_str()) != 0 ) {
        line_sender ls(lc);
        ls << "406 error_dir Cannot change to directory '" << escape(dirname) << "'.\n";
    } else {
        std::ostringstream out;
        out << "207 status_cwd " << dirname << '\n';
        ls_sort->send_all(out.str());
    }
}

// ########################################################################

static void command_clear(line_channel*, const std::string&, void* hists)
{
    if ( !hists ) {
        LFLOG_ERROR << "User data 'hists' was not provided";
    }

    SharedHistograms *histograms = reinterpret_cast<SharedHistograms *>(hists);
    histograms->ResetAll();
    buffer_count = bad_buffer_count = 0;

    time_t now = time(0);
    char buf[1024];
    strftime(buf, sizeof(buf), "204 status_cleared %Y%m%d-%H%M%S\n", localtime(&now));
    ls_sort->send_all(buf);
}

// ########################################################################

static void command_dump(line_channel* lc, const std::string&, void* hists)
{


    time_t now = time(0);
    char filename[1024];
    strftime(filename, sizeof(filename), "dump-%Y%m%d-%H%M%S.root", localtime(&now));

    if ( !hists ) {
        line_sender ls(lc);
        ls << "401 error_file Could not write '" << filename << "'.\n";
        LFLOG_ERROR << "User data 'hists' was not provided";
    }
    auto histograms = reinterpret_cast<SharedHistograms *>(hists);
    RootWriter::Write(*histograms, filename, "XIAonline");
    ls_sort->send_all("203 status_dumped all\n");
}

// ########################################################################

static void broadcast_bufcount(line_channel* lc=nullptr)
{
    std::ostringstream o;
    o << "101 bufs " << buffer_count <<' '<< bad_buffer_count;
      //<<' '<< evtbldr->GetAverageLength() <<'\n';
    send_1_or_all(o.str(), lc);
}

// ########################################################################

static void cb_connected(line_channel* lc, void*)
{
    std::cout << "acq_sort: new client" << std::endl;
    broadcast_bufcount(lc);
    command_status_cwd(lc, "connect", nullptr);
}

// ########################################################################

static void cb_disconnected(line_channel*, void*)
{
    std::cout << "acq_sort: client disconnected" << std::endl;
}

// ########################################################################

static void broadcast_gainshift(line_channel* lc=0)
{
    std::ostringstream o;
    //o << "201 status_gain " << format_gainshift(*GetCalibration()) << '\n';

    send_1_or_all(o.str(), lc);
}

// ########################################################################

static void command_status_gain(line_channel* lc, const std::string&, void*)
{
    broadcast_gainshift(lc);
}

// ########################################################################

static void command_gain(line_channel* lc, const std::string& cmd, void*)
{
    /*const std::string filename = cmd.substr(5);
    if( !read_gainshifts(*GetCalibration(), filename) ) {
        line_sender ls(lc);
        ls << "402 error_file Problem reading gain/shift from '"<<filename<<"'.\n";
    } else {
        broadcast_gainshift();
    }*/
}

// ########################################################################

double avr_event_length(const std::vector<std::vector<Entry_t>>& events) {
    size_t tot_event_length = 0;
    size_t num_events = events.size();
    for ( auto& event : events ) {
        tot_event_length += event.size();
    }
    return static_cast<double>(tot_event_length)/static_cast<double>(num_events);
}

// ########################################################################

int main (int argc, char* argv[])
{
    std::ifstream config_file;
    if ( argc == 1 ) {
        config_file.open("config.yml");
    } else if ( argc == 2 ) {
        config_file.open(argv[1]);
    } else {
        std::cerr << "acq_sort runs with no or one parameter" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Set up a log-handler to stdout
    logfault::LogManager::Instance().AddHandler(std::make_unique<logfault::StreamHandler>(std::clog, logfault::LogLevel::INFO));
    ::shm_unlink("/XIAonline");

    SharedHistograms histograms = SharedHistograms::Create("XIAonline", size_t(1) << 33, 256);

    // Set up logger instance
    UserConfiguration config = UserConfiguration::FromFile(config_file);

    signal(SIGINT, keyb_int); // set up interrupt handler (Ctrl-C)
    signal(SIGPIPE, SIG_IGN);


    io_select ioc;
    struct command_cb::command sort_commands[] = {
        {"quit",        0,  command_quit,       0},
        {"clear",       0,  command_clear,      reinterpret_cast<void*>(&histograms)},
        {"dump",        0,  command_dump,       reinterpret_cast<void*>(&histograms)},
        {"status_cwd",  0, command_status_cwd,  0},
        {"change_cwd",  1, command_change_cwd,  0},
        {"gain",        1, command_gain,        0},
        { "status_gain",0, command_status_gain, 0 },
        {0, 0, 0, 0}
    };

    ls_sort = new line_server(ioc, 32010, "acq_sort",
                              new line_cb(cb_connected), new line_cb(cb_disconnected),
                              new command_cb(sort_commands, "407 error_cmd"));

    // attach shared databuffer segment (written by engine)
    unsigned int *engine_shm = engine_shm_attach(false);
    if( !engine_shm ) {
        std::cerr << "Failed to attach engine shm." << std::endl;
        exit(EXIT_FAILURE);
    }
    const volatile int* time_us = (int*)&engine_shm[ENGINE_TIME_US];
    const volatile int* time_s  = (int*)&engine_shm[ENGINE_TIME_S ];
    const volatile unsigned int* data    = engine_shm + engine_shm[ENGINE_DATA_START];
    const volatile unsigned int  datalen = engine_shm[ENGINE_DATA_SIZE];
    const volatile unsigned int* first_header = (unsigned int*)&engine_shm[ENGINE_FIRST_HEADER];

    // Tasks
    Task::InputQueue_t input_queue;
    Task::Unpacker unpacker(input_queue, config.GetConfigManager());
    Task::Buffer buffer(unpacker.GetQueue());
    Task::Splitter splitter(buffer.GetQueue(), config.GetSplitTime());
    Task::Singles::Sorter ssort(histograms, splitter.GetQueue(), config);
    Task::Trigger trigger(ssort.GetQueue(), config);
    Task::Coincidence::Sorter csort(histograms, trigger.GetQueue(), config);

    // Declare the sorting routine
    ThreadPool<std::thread> pool;

    pool.AddTask(&unpacker);
    pool.AddTask(&buffer);
    pool.AddTask(&splitter);
    pool.AddTask(&ssort);
    pool.AddTask(&trigger);
    pool.AddTask(&csort);

    bool error = false;
    int last_tus=0;
    int last_t=0;

    while ( leaveprog == 'n' ){
        const int tus = *time_us;
        const int ts = *time_s;
        error = false;

        if (tus != 0 && ts != 0) {
            if (ts > last_t || (ts == last_t && tus > last_tus)){
                last_t = ts;
                last_tus = tus;
                input_queue.push(std::vector(data+(*first_header), data+datalen-(*first_header)));

                //data_p = unpacker->ParseBuffer(data+(*first_header), datalen-(*first_header), error);
                //sort_singles(data_p);
                ++buffer_count;
                //evtbldr->SetBuffer(data_p);
                if ( error )
                    continue; // We just skip if there was a problem!

                /*while (true){
                    evt_status = evtbldr->Next(event);
                    if (evt_status == EventBuilder::ERROR){
                        ++bad_buffer_count;
                        break;
                    }
                    sort_coincidence(event);
                    if (evt_status == EventBuilder::END)
                        break;
                } */

                broadcast_bufcount(0);
            }
        }

        // check for commands, wait up to 0.02ms
        struct timeval timeout = { 0, 20 };
        ioc.run(&timeout);
    }

    // detach shared memory
    engine_shm_detach();
}
