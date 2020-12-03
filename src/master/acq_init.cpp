
#include "acq_gui.h"
#include "acq_log.h"
#include "m_engine.h"
#include "m_sort.h"
#include "io_xtapp.h"
#include "run_command.h"

#include <iostream>

#include <locale.h>
#include <signal.h>
#include <stdlib.h>

#include "debug.h"

command_list* commands = 0;

static io_control* ioc = 0;

// ########################################################################

void connect_to_engine()
{
    if( !m_engine_connect(*ioc) )
        log_message(LOG_ERR, "Could not connect to engine.\n");
    else
        log_message(LOG_INFO, "Connected to engine.\n");
}

// ########################################################################

void connect_to_sort() {
    if( !m_sort_connect(*ioc) )
        log_message(LOG_ERR, "Could not connect to sort.\n");
    else
        log_message(LOG_INFO, "Connected to sort.\n");
}

// ########################################################################

int main(int argc, char *argv[])
{
    signal(SIGPIPE, SIG_IGN);

    if( gui_setup(argc, argv)!=0 ) {
        std::cerr << "**** ERROR **** GUI setup failed." << std::endl;
        ::exit(EXIT_FAILURE);
    }
    setlocale(LC_NUMERIC, "C");

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

    ioc = new io_xtapp(app_context);

    connect_to_engine();
    connect_to_sort();

    XtAppMainLoop(app_context);

    m_engine_disconnect();
    m_sort_disconnect();

    delete ioc;
    delete commands;

    return 0;
}
