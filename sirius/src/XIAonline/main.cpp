
#include <iostream>

#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>

#include "engine_shm.h"
#include "net_control.h"
#include "sort_spectra.h"
#include "utilities.h"


#include "XIARoutine.h"
#include "Event.h"
#include "Unpacker.h"
#include "Event_builder.h"
#include "spectrum_rw.h"
#include "Sort_Funct.h"

char leaveprog='n';
static int buffer_count=0,bad_buffer_count=0;

static line_server *ls_sort = 0;

static Unpacker *unpacker;
static EventBuilder *evtbldr;


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
        lc->send(message.c_str());
    else
        ls_sort->send_all(message.c_str());
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

static void command_clear(line_channel*, const std::string&, void*)
{
    for(int i=1; sort_spectra[i].name; ++i) {
        const sort_spectrum_t* s = &sort_spectra[i];
        bzero(s->ptr, s->ydim*s->xdim*sizeof(*s->ptr));
    }
    buffer_count = bad_buffer_count = 0;

    time_t now = time(0);
    char buf[1024];
    strftime(buf, sizeof(buf), "204 status_cleared %Y%m%d-%H%M%S\n", localtime(&now));
    ls_sort->send_all(buf);
}

// ########################################################################

static void command_dump(line_channel* lc, const std::string&, void*)
{
    time_t now = time(0);
    char dirname[1024];
    strftime(dirname, sizeof(dirname), "dump-%Y%m%d-%H%M%S", localtime(&now));
    if( mkdir(dirname, 0755) != 0 ) {
        line_sender ls(lc);
        ls << "401 error_file Could not create '" << dirname << "'.\n";
        return;
    }
    std::string dn = dirname;

    std::vector<std::string> args;
    bool all_okay = true;
    for(int i=1; sort_spectra[i].specno; ++i) {
        char filename[1024];
        snprintf(filename, sizeof(filename), "%s/%s", dirname, sort_spectra[i].name);
        if( !dump_spectrum(&sort_spectra[i], 0, filename) ) {
            all_okay = false;
            line_sender ls(lc);
            ls << "401 error_file Could not write '" << filename << "'.\n";
        } else {
            args.push_back(filename);
        }
    }
    if( !args.empty() ) {
        args.push_back(dn+"/"+dn+".root");
        //commands->run("mama2root", args);
    }
    if( all_okay )
        ls_sort->send_all("203 status_dumped all\n");
}

// ########################################################################

static void broadcast_bufcount(line_channel* lc=0)
{
    std::ostringstream o;
    o << "101 bufs " << buffer_count <<' '<< bad_buffer_count
      <<' '<< evtbldr->GetAverageLength() <<'\n';
    send_1_or_all(o.str(), lc);
}

// ########################################################################

static void cb_connected(line_channel* lc, void*)
{
    std::cout << "acq_sort: new client" << std::endl;
    broadcast_bufcount(lc);
    command_status_cwd(lc, "connect", 0);
}

// ########################################################################

static void cb_disconnected(line_channel*, void*)
{
    std::cout << "acq_sort: client disconnected" << std::endl;
}


int main (int argc, char *[])
{

    if( argc != 1 ) {
        std::cerr << "acq_sort runs without parameters" << std::endl;
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, keyb_int); // set up interrupt handler (Ctrl-C)
    signal(SIGPIPE, SIG_IGN);


    io_select ioc;
    struct command_cb::command sort_commands[] = {
        {"quit",        0,  command_quit,       0},
        {"clear",       0,  command_clear,      0},
        {"dump",        0,  command_dump,       0},
        {"status_cwd",  0, command_status_cwd,  0},
        {"change_cwd",  1, command_change_cwd,  0},
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


    // Attach shared memory
    if ( !spectra_attach_all(true) ){
        std::cerr << "Failed to attach shm spectra." << std::endl;
        exit(EXIT_FAILURE);
    }

    // Declare the sorting routine
    unpacker = new Unpacker;
    evtbldr = new EventBuilder;

    EventBuilder::Status evt_status;

    Event event;
    std::vector<word_t> data_p;

    bool error = false;
    int last_tus=0;
    int last_t=0;

    while ( leaveprog == 'n' ){
        const int tus = *time_us;
        const int ts = *time_s;

        if (tus != 0 && ts != 0) {
            if (ts > last_t || (ts == last_t && tus > last_tus)){
                last_t = ts;
                last_tus = tus;

                data_p = unpacker->ParseBuffer(data, datalen, error);
                sort_singles(data_p);
                ++buffer_count;
                evtbldr->SetBuffer(data_p);

                while (true){
                    evt_status = evtbldr->Next(event);
                    if (evt_status == EventBuilder::ERROR){
                        ++bad_buffer_count;
                        break;
                    }
                    sort_coincidence(event);
                    if (evt_status == EventBuilder::END)
                        break;
                }

                broadcast_bufcount(0);
            }
        }

        // check for commands, wait up to 0.02ms
        struct timeval timeout = { 0, 20 };
        ioc.run(&timeout);
    }

    // detach shared memory
    engine_shm_detach();
    spectra_detach_all();
}
