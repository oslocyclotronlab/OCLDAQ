
#include "engine_shm.h"
#include "net_control.h"
#include "utilities.h"
#include "run_command.h"

#include "WriteTerminal.h"
#include "XIAControl.h"
#include "functions.h"

//#include "mainwindow.h"
#include "xiainterface.h"
#include "xiainterface2.h"
#include <QApplication>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

#include <thread>

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstdio>
#include <sys/time.h>
#include <unistd.h>

#include <xiaconfigurator.h>

#if _FILE_OFFSET_BITS != 64
#error must compile with _FILE_OFFSET_BITS == 64
#endif

//static const int MAX_BUFFER_COUNT = 8192; // max 2GB files
static const int MAX_BUFFER_COUNT = 16384; // max 2GB files

char leaveprog='n'; // leave program (Ctrl-C / INT)
static bool stopped = true;
static int buffer_count = -1;
static float buffer_rate = 0;
static std::string output_filename;
static FILE* output_file=0;
static unsigned int datalen_char = 1;
static timeval last_time = { 0, 0 };

static line_server* ls_engine = 0;

static WriteTerminal termWrite;
static XIAControl *xiacontr;
static std::thread gui_thread;
static int gui_is_running;
static int globargc;
static char **globargv;
command_list* commands = 0;
XIAConfigurator *config = nullptr;

#ifndef OFFLINE
#define OFFLINE false
#endif // OFFLINE

// ########################################################################
// ########################################################################

void keyb_int(int sig_num)
{
    if (sig_num == SIGINT) {
        termWrite.Write("\n\nLeaving...\n");
        leaveprog = 'y';
    }
}

// ########################################################################
// ########################################################################


int GUI_thread(int nmod)
{
    if ( config )
        config->show();
    return 0;
}



static void close_file()
{
    if( output_file ) {
        char tmp[2048];
        snprintf(tmp, sizeof(tmp), "engine: file '%s' was closed.\n", output_filename.c_str());
        fflush(output_file);
        fclose(output_file);
        output_file = nullptr;
    }
    buffer_count = 0;
}

// ########################################################################

static bool open_file()
{
    close_file();

    if( output_filename.empty() )
        return true;

    output_file = fopen(output_filename.c_str(), "ab");
    if( !output_file ) {
        std::ostringstream out;
        out << "501 error_file Could not open '"
            << escape(output_filename)
            << "' for append.\n";
        ls_engine->send_all(out.str());
        return false;
    } else {
        const long fs = ftell(output_file);
        buffer_count = fs / datalen_char;
        char tmp[2048];
        snprintf(tmp, sizeof(tmp), "engine: file '%s' (%d buffers) was opened.\n", output_filename.c_str(), buffer_count);
        termWrite.Write(tmp);
        return true;
    }
}

// ########################################################################

static void do_stop()
{
    xiacontr->XIA_end_run(output_file, output_filename.c_str());
    close_file();
    stopped = true;
    last_time.tv_sec = last_time.tv_usec = 0;

    ls_engine->send_all("201 status_stopped\n");

    std::cout << "sleeping 1s to avoid confusing bobcat... " << std::flush;
    sleep(1);
    std::cout << "done" << std::endl;
}

// ########################################################################

static bool do_change_output_file(const std::string& fname)
{
    if( fname.empty() || fname == output_filename )
        return false;

    // close the file if it exists
    close_file();

    // change output filename
    output_filename = fname;

    // inform about filename change
    std::ostringstream out;
    out << "203 output_file " << escape(output_filename) << '\n';
    ls_engine->send_all(out.str());

    if ( xiacontr )
        xiacontr->setFile(output_filename.c_str());

    // if started, try to open the new file, and stop if that fails
    if( !stopped ) {
        if( !open_file() ) {
            do_stop();
            return false;
        }
    }
    return true;
}

// ########################################################################

static bool change_output_file()
{
    std::string::size_type dot = output_filename.find_last_of(".");
    if( dot == 0 || dot == std::string::npos ) {
        std::cerr << "engine: dot not found in filename '" << output_filename
                  << "', will not change filename." << std::endl;
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

// ########################################################################
// ########################################################################

static void command_quit(line_channel* lc, const std::string&, void*)
{
    if( !stopped ) {
        lc->send("401 error_state Can only quit if stopped.\n");
    } else {
        close_file();
        leaveprog = 'y';
    }
}

// ########################################################################

static void command_stop(line_channel* lc, const std::string&, void*)
{
    if( stopped ) {
        return lc->send("402 error_state Already stopped.\n");
    } else {
        do_stop();
    }
}

// ########################################################################

static void broadcast_buffer_count()
{
    std::ostringstream out;
    out << "101 buffer_count " << buffer_count << ' ' << buffer_rate << '\n';
    ls_engine->send_all(out.str());
}

// ########################################################################

static void command_start(line_channel* lc, const std::string&, void*)
{
    if( !stopped ) {
        lc->send("403 error_state Already started.\n");
        return;
    }

    if( !open_file() )
        return;
    xiacontr->XIA_start_run();
    if( !xiacontr->XIA_check_status() ) {
        close_file();
        lc->send("502 error_vme Could not connect to VME - eventbuilder stopped?.\n");
        return;
    }

    stopped = false;

    ls_engine->send_all("202 status_started\n");
    broadcast_buffer_count();
}

// ########################################################################

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

// ########################################################################

static void command_status(line_channel* lc, const std::string&, void*)
{
    lc->send(stopped ? "201 status_stopped\n" : "202 status_started\n");
    if( !output_filename.empty() ) {
        line_sender ls(lc);
        ls << "203 output_file " << escape(output_filename) << '\n';
    } else  {
        lc->send("204 output_none\n");
    }
    command_output_get_dir(lc, "status", 0);
    if( !stopped ) {
        line_sender ls(lc);
        ls << "101 buffer_count " << buffer_count << '\n';
    }
}

// ########################################################################

static void command_launch_GUI(line_channel* lc, const std::string&, void*)
{
    // We need to open the GUI. First check if the GUI is in fact open, if so
    // we will need to give information back to the user that we have the GUI
    // open.

    // Check if we are running, if still in 'main loop' we do not want to try to join the threads. Return something else for now...
    if ( gui_is_running == 1 ) {
            lc->send("403 error_state GUI is already launched");
            return;
    }


    if ( gui_thread.joinable() ){
        gui_thread.join();
    }

    // If we reach this point we can safely launch the gui :D
    gui_thread = std::thread(GUI_thread, xiacontr->GetNumMod());

}

// ########################################################################

static void command_reload(line_channel *lc, const std::string&, void *)
{
    command_launch_GUI(lc,"",nullptr);
    /*if ( !stopped ){
        lc->send("403 error_state cannot reload while running.\n");
        return;
    }

    if (xiacontr->XIA_reload()){
        lc->send("504 error_xia Couldn't reload parameters\n");
        return;
    }*/
}


// ########################################################################

static void command_output_none(line_channel* lc, const std::string&, void*)
{
    if( output_filename.empty() ) {
        lc->send("404 error_cmd 'none' output already selected.\n");
        return;
    }

    close_file();
    output_filename = "";
    if ( xiacontr )
        xiacontr->setFile(output_filename.c_str());

    ls_engine->send_all("204 output_none\n");
}

// ########################################################################

static void command_output_file(line_channel* lc, const std::string& line, void*)
{
    const std::string fname = line.substr(12);
    if( !do_change_output_file(fname) ) {
        line_sender ls(lc);
        ls << "405 error_file Cannot select file '" << escape(fname) << "'.\n";
    }
}

// ########################################################################

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

// ########################################################################
// ########################################################################

static void cb_connected(line_channel* lc, void*)
{
    termWrite.Write("engine: new client\n");
    command_status(lc, "status", 0);
}

static void cb_disconnected(line_channel*, void*)
{
    termWrite.Write("engine: client disconnected\n");
}

// ########################################################################
// ########################################################################

int main_engine(int argc, char* argv[])
{

    commands = new command_list();
    if( (commands->read("acq_master_commands.txt")) ) {
        std::cerr << "Using commands from acq_master_commands.txt." << std::endl;
    } else {
        std::cerr << "Using default commands." << std::endl;
        commands->read_text(
                "mama     = xterm -bg moccasin -fg black -geometry 80x25+5-60 -e mama\n"
                "rupdate  = rupdate\n"
                "loadsort = xterm -bg khaki -fg black -geometry 100x25-50+0 -e loadsort\n"
                "readme   = echo\n"
                "manual   = firefox http://ocl.uio.no/sirius/\n"
                "sort     = xterm -e acq_sort\n"
                "engine   = xterm -e usb-engine\n"
                "elog     = echo\n"
        );
    }

    io_select ioc;

    static command_cb::command engine_commands[] = {
            { "quit",        false, command_quit,        0 },
            { "stop",        false, command_stop,        0 },
            { "start",       false, command_start,       0 },
            { "output_none", false, command_output_none, 0 },
            { "output_file", true,  command_output_file, 0 },
            { "output_get_dir", false, command_output_get_dir, 0 },
            { "output_dir",  true,  command_output_dir,  0 },
            { "status",      false, command_status,      0 },
            { "reload",      false, command_reload,      0 },
            { 0, 0, 0, 0 }
    };

    try {
        ls_engine = new line_server
                (ioc, 32009, "engine",
                 new line_cb(cb_connected), new line_cb(cb_disconnected),
                 new command_cb(engine_commands, "407 error_cmd"));
    } catch ( const std::exception &ex ){
        std::cerr << ex.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    // attach shared memory and initialize some variables
    unsigned int* buffer  = engine_shm_attach(true);
    if( !buffer ) {
        std::cerr << "engine: Failed to attach shared memory." << std::endl;
        exit(EXIT_FAILURE);
    }
    unsigned int* time_us       = &buffer[ENGINE_TIME_US];
    unsigned int* time_s        = &buffer[ENGINE_TIME_S ];
    unsigned int* data          = buffer + buffer[ENGINE_DATA_START];
    unsigned int* first_header  = &buffer[ENGINE_FIRST_HEADER];
    const unsigned int datalen  = buffer[ENGINE_DATA_SIZE];
    /*const unsigned int*/ datalen_char = datalen*sizeof(int);

    // main loop
    while( leaveprog == 'n' ) {
        if( !stopped ) {
            if ( xiacontr->XIA_check_buffer(datalen) ) {
                // a buffer is available; reset timestamp
                *time_us = *time_s = 0;
                // transfer the buffer
                if( !xiacontr->XIA_fetch_buffer(data, datalen, first_header) ) {
                    // the buffer was not transferred completely, stop
                    do_stop();
                } else {

                    // write actual timestamp
                    timeval t{};
                    gettimeofday(&t, 0);
                    *time_us = t.tv_usec;
                    *time_s  = t.tv_sec;

                    // write buffer
                    if( output_file ) {
                        unsigned int w = fwrite(data, 1, datalen_char, output_file);
                        if( w != datalen_char ) {
                            ls_engine->send_all("503 error_file Write error, closing file and stopping.\n");
                            do_stop();
                        }
                    }

                    // calculate buffer rate
                    if( last_time.tv_sec!=0 && last_time.tv_usec!=0 ) {
                        // but only if this is not the first buffer
                        buffer_rate = (t.tv_sec + 1e-6*t.tv_usec)
                                      -(last_time.tv_sec + 1e-6*last_time.tv_usec);
                        if( buffer_rate>0 )
                            buffer_rate = 1/buffer_rate;
                        else
                            buffer_rate = 999999;
                    } else {
                        buffer_rate = 0;
                    }
                    last_time = t;


                    // send message about new buffer count
                    buffer_count += 1;
                    broadcast_buffer_count();
                    if( output_file && buffer_count == MAX_BUFFER_COUNT )
                        change_output_file();
                }
                continue;
            }

            if( !xiacontr->XIA_check_status() )
                do_stop();
        }
        struct timeval timeout = { 0, 250 };
        ioc.run(&timeout);
    }

    engine_shm_detach();


    delete commands;
    return 0;

}

// ########################################################################
// ########################################################################

int main_gui(int nmod, QApplication &app, XIAConfigurator &c)
{
    c.show();
    /*while ( leaveprog == 'n' )
        app.processEvents();
    return 0;*/
    return app.exec();
}

// ########################################################################
// ########################################################################

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    unsigned short PXIMapping[PRESET_MAX_MODULES];
    for (unsigned short & mapping : PXIMapping)
        mapping = 0;

    // If there is a mapping given in the command line we will read that, if not we will try
    // to determine the slot mapping our self.
    if ( argc == 1 ){
        try {
            auto mapping = ReadSlotMap();
            if ( mapping.size() >= PRESET_MAX_MODULES ){
                std::string errmsg = "Too many PCI devices found, found " + std::to_string(mapping.size());
                throw std::runtime_error(errmsg);
            }
            int set = 0;
            for ( auto &entry : mapping ){
                PXIMapping[set++] = entry;
            }
        } catch ( std::exception &ex ){
            std::cerr << "Could not determine PLX slot mapping, got error " << ex.what() << std::endl;
            std::cerr << "Try with manual PXI slot mapping, e.g.:" << std::endl;
            std::cerr << argv[0] << " 2 3 4 5" << std::endl;
            exit(EXIT_FAILURE);
        }
    } else {
        for (int i = 1 ; i < argc ; ++i)
            PXIMapping[i] = atoi(argv[i]);
    }

    signal(SIGINT, keyb_int); // set up interrupt handler (Ctrl-C)
    signal(SIGPIPE, SIG_IGN);

    // sleep a little to avoid repeated timestamps
    usleep(10);

    xiacontr = new XIAControl(&termWrite, PXIMapping);

    // We will now boot before anything else will happend.
    if ( !xiacontr->XIA_boot_all(OFFLINE) )
        leaveprog = 'y';

    auto nmod = xiacontr->GetNumMod();
    XIAInterfaceAPI2 interface(nmod);
    XIAConfigurator configurator(&interface);

    // Now we are ready to start the two threads, this will launch the settings window!
    auto engine_thread = std::thread(main_engine, argc, argv);
    auto r = main_gui(nmod, app, configurator);

    if ( engine_thread.joinable() ) engine_thread.join();

    return r;
}

// ########################################################################
// ########################################################################

/* for emacs */
/*** Local Variables: ***/
/*** indent-tabs-mode: nil ***/
/*** End: ***/