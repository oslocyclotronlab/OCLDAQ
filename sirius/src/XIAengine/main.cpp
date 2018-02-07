
// STL C++ libraries
#include <string>
#include <queue>

// Multi-thread libraries
#include <thread>

// C libraries
#include <cstdio>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "globals.h"
#include "XIAreader.h"


#include "net_control.h"
#include "engine_shm.h"
#include "utilities.h"

char leaveprog = 'n';
bool stopped = true;
bool stopXIA = false;

EventMerger merger;
WriteTerminal termWrite;

static const int max_buffer_count = 8192; // Max number of buffers.

static int buffer_count = -1;
static float buffer_rate = 0;
static std::string output_filename;
static FILE* output_file=0;
static unsigned int datalen_char = 1;
static timeval last_time = { 0,  0 };
static line_server *ls_engine = 0;

static std::queue<Event_t> data_waiting;
static std::vector<uint32_t> overflow;
static unsigned int data_in_queue;

static std::thread merge_thread;
static std::thread readout_thread;


void keyb_int(int sig_num)
{
    if (sig_num == SIGINT) {
        printf("\n\nLeaving...\n");
        leaveprog = 'y';
    }
}

static void Flush_overflow()
{
    if (overflow.size() > 0){
        if ( output_file ){
            uint32_t *tmp = new uint32_t[overflow.size()];
            for (unsigned int i = 0 ; i < overflow.size() ; ++i){
                tmp[i] = overflow[i];
            }
            if (fwrite(tmp, overflow.size(), sizeof(uint32_t), output_file) != overflow.size()){
                // do something...
            }
            overflow.clear();
        }
    }
    return;
}

static void Flush_merger_to_disk()
{
    // Will flush everything we have in merger to our FIFO
    // then write everything to disk.

    // First we will flush the overflow vector.
    Flush_overflow();

    // Flush merger
    std::vector<Event_t> from_mrg = merger.FlushData();

    for (unsigned int i = 0 ; i < from_mrg.size() ; ++i){
        data_waiting.push(from_mrg[i]);
        data_in_queue += from_mrg[i].size_raw;
    }
    int size = data_in_queue;
    uint32_t *tmp = new uint32_t[size];
    Event_t currentevent;
    while ( !data_waiting.empty() ){
        currentevent = data_waiting.front();
        for (int i = 0 ; i < currentevent.size_raw ; ++i){
            tmp[i] = currentevent.raw_data[i];
        }
        data_in_queue -= currentevent.size_raw;
        data_waiting.pop();
    }

    // Writing data to disk
    if (fwrite(tmp, size , sizeof(uint32_t), output_file) != data_in_queue){
        // do something
    }

    data_in_queue = 0;
    return;
}


static void close_file()
{
    if ( output_file ) {
        termWrite.Write("XIAengine: file '");
        termWrite.Write(output_filename.c_str());
        termWrite.Write("' was closed.\n");
        fflush(output_file);
        fclose(output_file);
        output_file = 0;
    }
    buffer_count = 0;
}

static bool open_file()
{
    close_file();

    if ( output_filename.empty() )
        return true;

    output_file = fopen(output_filename.c_str(), "ab");
    if ( !output_file ){
        std::ostringstream out;
        out << "501 error_file Could not open '"
            << escape(output_filename)
            << "' for append.\n";
        ls_engine->send_all(out.str());
        return false;
    } else {
        const long fs = ftell(output_file);
        buffer_count = fs / datalen_char;

        termWrite.Write("XIAengine: file '");
        termWrite.Write(output_filename.c_str());
        termWrite.Write("' (");
        termWrite.Write(std::to_string(buffer_count).c_str());
        termWrite.Write(" buffers) was opened.\n");
        return true;
    }
}

static void do_stop()
{
    stopXIA = true;
    termWrite.Write("Waiting for the readout thread to finish...");
    readout_thread.join();
    termWrite.Write(" done\n");

    if ( output_file ) // Flush everything in the merger to file
        Flush_merger_to_disk();

    stopped = true;
    close_file();
    last_time.tv_sec = last_time.tv_usec = 0;
    ls_engine->send_all("201 status_stopped\n");
    return;
}

static bool do_change_output_file(const std::string &fname)
{
    if ( fname.empty() || fname == output_filename )
        return false;

    // close the file if exists.
    close_file();

    // change output filename
    output_filename = fname;

    // Inform about filename change.
    std::ostringstream out;
    out << "203 output_file " << escape(output_filename) << '\n';
    ls_engine->send_all(out.str());

    // If started, try to open the new file and stop if that fails.
    if ( !stopped ){
        if ( !open_file() ){
            do_stop();
            return false;
        }
    }
    return true;
}

static bool change_output_file()
{
    Flush_overflow();
    std::string::size_type dot = output_filename.find_last_of(".");
        if( dot == 0 || dot == std::string::npos ) {
            termWrite.WriteError("engine: dot not found in filename '");
            termWrite.WriteError(output_filename.c_str());
            termWrite.WriteError("', will not change filename.\n");
            return false;
        }

        std::string new_filename = output_filename;

        const std::string extension = "-big-";
        std::string::size_type ext = output_filename.find(extension);
        if( ext != std::string::npos && (ext+extension.size()+3 == dot) ) {
            // already have extension; need to find new number
            std::string num = output_filename.substr(ext+extension.size(), 3);
            int n = 0;
            for(int i=0; i<3; ++i) {
                if( num[i]>='0' && num[i]<='9' )
                    n = 10*n + (num[i]-'0');
            }
            if( n<0 || n>998 )
                return false;
            n += 1;
            new_filename.replace(ext+extension.size(), 3, ioprintf("%03d", n));
            //std::cerr << "file with 2+ extension: '" << new_filename << "'" << std::endl;
        } else {
            // no extension yet,
            const std::string i = extension + "000";
            new_filename.insert(dot, i);
            //std::cerr << "file with 1st extension: '" << new_filename << "'" << std::endl;
        }
        // need to check if new_filename exists and fail if yes; otherwise
        // files could contain data in confused time order
        if( file_exists(new_filename) ) {
            //std::cerr << "file '" << new_filename << "'exists" << std::endl;
            return false;
        }

        return do_change_output_file(new_filename);
}

static void broadcast_buffer_count()
{
    std::ostringstream out;
    out << "101 buffer_count " << buffer_count << ' ' << buffer_rate << '\n';
    ls_engine->send_all(out.str());
}

static void command_quit(line_channel* lc, const std::string&, void*)
{
    if ( !stopped ) {
        lc->send("401 error_state Can only quit if stopped.\n");
    } else {
        //close_file();
        leaveprog='y';
    }
}

static void command_stop(line_channel *lc, const std::string &, void *)
{
    if ( stopped ) {
        lc->send("402 error_state Already stopped.\n");
    } else {
        do_stop();
    }
}

static void command_start(line_channel *lc, const std::string &, void *)
{
    if ( !stopped ) {
        lc->send("403 error_state Already started.\n");
        return;
    }

    if ( !stopXIA ){
        stopXIA = true;
        if (readout_thread.joinable())
            readout_thread.join();
    }

    overflow.clear();
    while (!data_waiting.empty()){
        data_waiting.pop();
    }
    data_in_queue = 0;

    if ( !open_file() )
        return;

    if ( !ReadSettingsFile("config.cfg") )
        return;
    readout_thread = std::thread(StartReadout, 16384);
}

static void command_output_none(line_channel* lc, const std::string&, void*)
{
    if( output_filename.empty() ) {
        lc->send("404 error_cmd 'none' output already selected.\n");
        return;
    }

    close_file();
    output_filename = "";

    ls_engine->send_all("204 output_none\n");
}

static void command_output_file(line_channel* lc, const std::string& line, void*)
{
    const std::string fname = line.substr(12);
    if( !do_change_output_file(fname) ) {
        line_sender ls(lc);
        ls << "405 error_file Cannot select file '" << escape(fname) << "'.\n";
    }
}

static void command_output_dir(line_channel* lc, const std::string& line, void*)
{
    if( !stopped ) {
        line_sender ls(lc);
        ls << "406 error_dir Cannot change directory while started.\n";
        return;
    }
    const std::string dirname = line.substr(11);
    if( chdir(dirname.c_str()) != 0 ) {
        line_sender ls(lc);
        ls << "406 error_dir Cannot change to directory '" << escape(dirname) << "'.\n";
    } else {
        std::ostringstream out;
        out << "205 output_dir " << dirname << '\n';
        ls_engine->send_all(out.str());
    }
}

static void command_output_get_dir(line_channel* lc, const std::string&, void*)
{
    char cwd[1024];
    if( !getcwd(cwd, sizeof(cwd)) ) {
        line_sender ls(lc);
        if( errno == ENOENT ) {
            ls << "205 output_dir -unlinked-\n";
        } else {
            ls << "407 error_dir Cannot get current directory.\n";
        }
    } else {
        line_sender ls(lc);
        ls << "205 output_dir " << cwd << '\n';
    }
}

static void command_status(line_channel *lc, const std::string&, void *)
{
    lc->send(stopped ? "201 status_stopped\n" : "202 status_started\n");
    if ( !output_filename.empty() ){
        line_sender ls(lc);
        ls << "203 output_file " << escape(output_filename) << '\n';
    } else {
        lc->send("204 output_none\n");
    }
    command_output_get_dir(lc, "status", 0);
    if ( !stopped ) {
        line_sender ls(lc);
        ls << "101 buffer_count " << buffer_count << '\n';
    }
}


static void cb_connected(line_channel* lc, void*)
{
    termWrite.Write("engine: new client\n");
    command_status(lc, "status", 0);
}

static void cb_disconnected(line_channel*, void*)
{
    termWrite.Write("engine: client disconnected\n");
}

static bool XIA_check_buffer()
{
    std::vector<Event_t> data = merger.GetData(1024);
    for (unsigned int i = 0 ; i < data.size() ; ++i){
        data_waiting.push(data[i]);
        data_in_queue += data[i].size_raw;
    }

    return (data_in_queue >= 0x8000);
}

static bool XIA_fetch_buffer(unsigned int *buffer, unsigned int bufsize)
{

    // If the size isn't 32k, then we return false...
    if (data_in_queue < bufsize)
        return false;
    size_t current_pos = 0;

    // Check if an event was split accross two buffers. Add that first!
    if (overflow.size() > 0){
        for (size_t i = 0 ; i < overflow.size() ; ++i)
            buffer[current_pos++] = overflow[i];
        overflow.clear();
    }

    Event_t thisevent;
    while (current_pos < bufsize){
        thisevent = data_waiting.front();
        if (current_pos + thisevent.size_raw < bufsize){
            for (int i = 0 ; i < thisevent.size_raw ; ++i){
                buffer[current_pos++] = thisevent.raw_data[i];
            }
        } else {
            for (size_t i = 0 ; i < bufsize - current_pos ; ++i){
                buffer[current_pos++] = thisevent.raw_data[i];
            }
            for (int i = bufsize - current_pos ; i < thisevent.size_raw ; ++i){
                overflow.push_back(thisevent.raw_data[i]);
            }
        }
        data_waiting.pop();
        data_in_queue -= thisevent.size_raw;
    }

    return true;
}


void ReadFromMerger()
{
    std::string write_str;
    while (leaveprog=='n'){
        std::vector<Event_t> getData = merger.GetData(150);
        write_str.clear();
        for (unsigned int i = 0 ; i < getData.size() ; ++i){
            write_str += getData[i].timestamp;
            write_str += ": ";
            write_str += getData[i].size_raw;
            write_str += "\n";
            termWrite.Write(write_str.c_str());
        }
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
}

static void start_merger()
{
    merger.SortThread();
}


int main(int argc, char* argv[])
{

    signal(SIGINT, keyb_int);
    signal(SIGPIPE, SIG_IGN);

    io_select ioc;

    static command_cb::command engine_commands[] = {
        { "quit",           false,  command_quit,           0},
        { "stop",           false,  command_stop,           0},
        { "start",          false,  command_start,          0},
        { "ouput_none",     false,  command_output_none,    0},
        { "output_file",    true,   command_output_file,    0},
        { "output_get_dir", false,  command_output_get_dir, 0},
        { "output_dir",     true,   command_output_dir,     0},
        { "status",         false,  command_status,         0},
        { 0, 0, 0, 0 }
    };

    ls_engine = new line_server
            (ioc, 32009, "engine",
             new line_cb(cb_connected), new line_cb(cb_disconnected),
             new command_cb(engine_commands, "407 error_cmd"));

    std::this_thread::sleep_for(std::chrono::microseconds(100));

    // Starting the merger thread.
    merge_thread = std::thread(start_merger);

    // Create the shared memory.
    uint32_t *buffer = engine_shm_attach(true);

    unsigned int* time_us = &buffer[ENGINE_TIME_US];
    unsigned int* time_s = &buffer[ENGINE_TIME_S];
    unsigned int* data = buffer + buffer[ENGINE_DATA_START];
    const unsigned int datalen = buffer[ENGINE_DATA_SIZE];

    while (leaveprog == 'n'){

        if ( !stopped ){

            if ( XIA_check_buffer() ){
                // 32k of data can be written :)

                *time_us = *time_s = 0;

                // Fill buffer into SHM object
                if ( !XIA_fetch_buffer(data, datalen) ){
                    // We did not recive the buffer, stop!
                    do_stop();
                } else {

                    timeval t;
                    gettimeofday(&t, NULL);
                    *time_us = t.tv_usec;
                    *time_s = t.tv_sec;

                    // Write buffer
                    if ( output_file ){
                        unsigned int w = fwrite(data, sizeof(unsigned int), datalen, output_file);
                        if (w != datalen){
                            ls_engine->send_all("503 error_file Write error, closing file and stopping.\n");
                            do_stop();
                        }
                    }

                    // Calculate buffer rate
                    if (last_time.tv_sec != 0 && last_time.tv_usec != 0){
                        buffer_rate = (t.tv_sec + 1e-6*t.tv_usec) - (last_time.tv_sec + 1e-6*last_time.tv_usec);
                        if (buffer_rate > 0)
                            buffer_rate = 1./buffer_rate;
                        else
                            buffer_rate = 1e9;
                    } else {
                        buffer_rate = 0;
                    }
                    last_time = t;

                    // Message about new buffer count
                    buffer_count += 1;
                    broadcast_buffer_count();
                    if ( output_file && buffer_count == max_buffer_count )
                        change_output_file();
                }
                continue;
            }
        }

        struct timeval timeout = { 0, 250 };
        ioc.run(&timeout);
    }

    engine_shm_detach();
    if (!merger.Terminate(1)){
        termWrite.WriteError("Unable to terminate the merger thread\n");
    }
    merge_thread.join();

    return 0;

}
