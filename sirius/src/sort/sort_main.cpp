
#include "engine_shm.h"
#include "net_control.h"
#include "user_routine_basic.h"
#include "run_command.h"
#include "sort_format.h"
#include "sort_spectra.h"
#include "spectrum_rw.h"
#include "utilities.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#if __APPLE__
#include <unistd.h>
#endif // __APPLE__

#if 0
static const char* PATH_GAINSHIFT  = "/Applications/sirius/data/gainshift.init";
static const char* PATH_TELEWIN    = "/Applications/sirius/data/telewin.init";
static const char* PATH_RANGE_DATA = "/Applications/sirius/data/RANGE.DATA";
#else
static const char* PATH_GAINSHIFT  = "gainshift.init";
static const char* PATH_TELEWIN    = "telewin.init";
static const char* PATH_RANGE_DATA = "RANGE.DATA";
#endif

char leaveprog='n'; // leave program (Ctrl-C / INT)

static int buffer_count=0, bad_buffer_count=0;

static UserRoutineBasic* user_routine;
extern UserRoutineBasic* user_routine_create();

static line_server* ls_sort = 0;

command_list* commands;

// ########################################################################
// ######################################################################## 

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

static void broadcast_gainshift(line_channel* lc=0)
{
    std::ostringstream o;
    o << "201 status_gain " << format_gainshift(user_routine->GetCalibration()) << '\n';

    send_1_or_all(o.str(), lc);
}

// ########################################################################

static void broadcast_telewin(line_channel* lc=0)
{
    std::ostringstream o;
    o << "202 status_telewin " << format_telewin(user_routine->GetCalibration()) << '\n';
    send_1_or_all(o.str(), lc);
}

// ########################################################################

static void broadcast_bufcount(line_channel* lc=0)
{
    std::ostringstream o;
    o << "101 bufs " << buffer_count <<' '<< bad_buffer_count
      <<' '<< unpack_get_avelen() <<'\n';
    send_1_or_all(o.str(), lc);
}
    
// ########################################################################

static void read_commands()
{
    if( commands->read("acq_master_commands.txt") ) {
        printf("Using commands from 'acq_master_commands.txt'.\n");
    } else {
        printf("Could not read commands from 'acq_master_commands.txt'.\n");
    }
}

// ########################################################################
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
        read_commands();
        std::ostringstream out;
        out << "207 status_cwd " << dirname << '\n';
        ls_sort->send_all(out.str());
    }
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
        commands->run("mama2root", args);
    }
    if( all_okay )
        ls_sort->send_all("203 status_dumped all\n");
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

static void command_gain(line_channel* lc, const std::string& cmd, void*)
{
    const std::string filename = cmd.substr(5);
    if( !read_gainshifts(user_routine->GetCalibration(), filename) ) {
        line_sender ls(lc);
        ls << "402 error_file Problem reading gain/shift from '"<<filename<<"'.\n";
    } else {
        broadcast_gainshift();
    }
}

// ########################################################################

static void command_status_gain(line_channel* lc, const std::string&, void*)
{
    broadcast_gainshift(lc);
}

// ########################################################################

static void command_range(line_channel* lc, const std::string& cmd, void*)
{
    const std::string filename = cmd.substr(6);
    if( !read_range_data(user_routine->GetCalibration(), filename) ) {
        line_sender ls(lc);
        ls << "403 error_file Problem reading range data from '"<<filename<<"'.\n";
    } else {
        ls_sort->send_all("205 status_range\n");
    }
}

// ########################################################################

static void command_telewin(line_channel* lc, const std::string& cmd, void*)
{
    const std::string filename = cmd.substr(8);
    if( !read_telewin(user_routine->GetCalibration(), filename) ) {
        line_sender ls(lc);
        ls << "404 error_cmd Problem reading telewin data from '" << filename << "'.\n";
    } else {
        broadcast_telewin();
    }
}

// ########################################################################

static void command_status_telewin(line_channel* lc, const std::string&, void*)
{
    broadcast_telewin(lc);
}

// ########################################################################

static void command_user_id(line_channel* lc, const std::string&, void*)
{
    line_sender ls(lc);
    ls << "206 status_user " << user_routine->GetId() << '\n';
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

// ########################################################################

int main(int argc, char* [])
{
    if( argc != 1 ) {
        std::cerr << "acq_sort runs without parameters" << std::endl;
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, keyb_int); // set up interrupt handler (Ctrl-C)
    signal(SIGPIPE, SIG_IGN);

    // remember startup time
    timeval last_time;
    gettimeofday(&last_time, 0);

    commands = new command_list();
    read_commands();

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

    if( !spectra_attach_all(true) ) {
        std::cerr << "Failed to attach shm spectra." << std::endl;
        exit(EXIT_FAILURE);
    }

    io_select ioc;

    struct command_cb::command sort_commands[] = {
        { "quit",           0, command_quit,           0 },
        { "clear",          0, command_clear,          0 },
        { "dump",           0, command_dump,           0 },
        { "gain",           1, command_gain,           0 },
        { "range",          1, command_range,          0 },
        { "telewin",        1, command_telewin,        0 },
        { "user_id",        0, command_user_id,        0 },
        { "status_gain",    0, command_status_gain,    0 },
        { "status_telewin", 0, command_status_telewin, 0 },
        { "status_cwd",     0, command_status_cwd,     0 },
        { "change_cwd",     1, command_change_cwd,     0 },
        //{ "status",  0, command_status,  0 },
        { 0, 0, 0, 0 }
    };
    ls_sort = new line_server
        (ioc, 32010, "acq_sort",
         new line_cb(cb_connected), new line_cb(cb_disconnected),
         new command_cb(sort_commands, "407 error_cmd"));

    user_routine = user_routine_create();
    read_gainshifts(user_routine->GetCalibration(), PATH_GAINSHIFT);
    read_telewin(user_routine->GetCalibration(), PATH_TELEWIN);
    read_range_data(user_routine->GetCalibration(), PATH_RANGE_DATA);

    unpacked_t unpacked;

    while( leaveprog=='n' ) {
        const int tus = *time_us, ts = *time_s;
        if( tus != 0 && ts != 0 ) {
            // data might be available, check time
            if( ts > last_time.tv_sec
                || (ts == last_time.tv_sec && tus > last_time.tv_usec ) )
            {
                // data timestamp written by engine is younger than our last time stamp -> sort data

                unpack_next_buffer();
                int unpack_err, nev=0;
                while(true) {
                    nev += 1;
                    unpack_err = unpack_next_event(data, datalen, &unpacked);
                    if( unpack_err == 1 )
                        break; // end-of-buffer
                    if( unpack_err ) {
                        std::cerr << "unpack error at "<<nev<<", skipping rest of buffer" << std::endl;
                        break;
                    }
                    if( tus != *time_us || ts != *time_s ) {
                        std::cerr << ioprintf("timestamp update %d.%06d => %d.%06d",
                                              ts, tus, *time_s, *time_us) << std::endl;
                        break;
                    }
                    user_routine->Sort(&unpacked);
                }
                
                buffer_count += 1;
                if( unpack_err > 1 )
                    bad_buffer_count += 1;

                broadcast_bufcount(0);

                gettimeofday(&last_time, 0);
            }
        }

        // check for commands, wait up to 0.5ms
        struct timeval timeout = { 0, 500 };
        ioc.run(&timeout);
    }

    // detach shared memory
    engine_shm_detach();
    spectra_detach_all();

    delete commands;

    return 0;
}

////////////////////////////////////////////////////////////////////////////
#if 0
    if( strlen(cmd)<=9 )
        return net_send_client("error_cmd dump <specname> </path/to/file>\n");

    const char* name_start = cmd + 5;
    const char* name_end = strchr(name_start, ' ');
    if( !name_end || name_end > name_start + 20 )
        return net_send_client("error_cmd dump: specname strange\n");

    const int name_len = name_end-name_start-1;
    char specname[128];
    strncpy(specname, name_start, name_len);
    specname[name_len+1] = 0;

    int id = spec_find_by_name(specname);
    if( id < 0 )
        return net_send_client("error_cmd dump: specname '%s' not found\n", specname);

#endif
////////////////////////////////////////////////////////////////////////////

/* for emacs */
/*** Local Variables: ***/
/*** c-basic-offset:4 ***/
/*** indent-tabs-mode: nil ***/
/*** compile-command:"make USER_ROUTINE_SRC=user_routine_simple.cpp" ***/
/*** End: ***/
