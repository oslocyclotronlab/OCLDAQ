
#include "m_sort.h"

#include "acq_gui.h"
#include "acq_log.h"
#include "net_control.h"
#include "run_command.h"
#include "utilities.h"

#include <iostream>

#include <stdlib.h>
#include <unistd.h>

#define NDEBUG
#include "debug.h"

extern command_list* commands;

static line_channel* lc_sort = 0;

static int sort_buffers=0, sort_errors=0;
static float sort_average_length=0;
static std::string sort_cwd;

// ########################################################################

template<typename T>
static T next(std::istream& in)
{
    T tmp;
    in >> tmp;
    return tmp;
}

static char space(int N, int i)
{
    return (i%N == N-1) ? '\n' : ' ';
}

static void m_sort_have_line(line_channel*, void*)
{
    const std::string l = lc_sort->get_line();
    //std::cout << "got line from sort: " << l << std::endl;

    std::istringstream line(l.c_str());
    int message_code = -1;
    std::string message_name = "?", remain;
    line >> message_code >> message_name;

    switch( message_code ) {
    case 101: // bufs count errors avelen
        line >> sort_buffers >> sort_errors >> sort_average_length;
        gui_update_state();
        break;

    case 201: { // status_gain data
        log_message(LOG_INFO, "sort: gain data have been updated.\n");

        std::ostringstream out;
        // read gain and shift values from string
        out << ioprintf("gain E:\n");
        for(int i=0; i<64; i++)
            out << ioprintf("%.3f%c", next<float>(line), space(8, i));
        out << ioprintf("gain DE:\n");
        for(int i=0; i<64; i++)
            out << ioprintf("%.3f%c", next<float>(line), space(8, i));
        out << ioprintf("E gain Ge:\n");
        for(int i=0; i<6; i++)
            out << ioprintf("%.3f ", next<float>(line));
        out << ioprintf("\nE gain NaI:\n");
        for(int i=0; i<32; i++)
            out << ioprintf("%.3f%c", next<float>(line), space(8, i));

        out << ioprintf("shift E:\n");
        for(int i=0; i<64; i++)
            out << ioprintf("%.3f%c", next<float>(line), space(8, i));
        out << ioprintf("shift DE:\n");
        for(int i=0; i<64; i++)
            out << ioprintf("%.3f%c", next<float>(line), space(8, i));
        out << ioprintf("E shift Ge:\n");
        for(int i=0; i<6; i++)
            out << ioprintf("%.3f ", next<float>(line));
        out << ioprintf("\nE shift NaI:\n");
        for(int i=0; i<32; i++)
            out << ioprintf("%.3f%c", next<float>(line), space(8, i));

        out << ioprintf("t shift Ge:\n");
        for(int i=0; i<6; i++)
            out << ioprintf("%.3f ", next<float>(line));
        out << ioprintf("\nt shift NaI:\n");
        for(int i=0; i<32; i++)
            out << ioprintf("%.3f%c", next<float>(line), space(8, i));

        log_message(LOG_INFO, "%s\n", out.str().c_str());

        break; }
    case 202: { // status_telewin
        log_message(LOG_INFO, "sort: telewin data have been updated.\n");

        std::ostringstream out;
        out << ioprintf("telewin:\n");
        for(int i=0; i<64; i++) {
            const int lo = next<int>(line), hi = next<int>(line);
            out << ioprintf("%2d=[%3d:%3d]%c", i, lo, hi, space(4, i));
        }
        log_message(LOG_INFO, "%s\n", out.str().c_str());

        break; }
    case 203: { // status_dumped %s
        std::string dumped;
        line >> dumped;
        log_message(LOG_INFO, "sort: %s spectra have been dumped.\n", dumped.c_str());
        break; }
    case 204: { // status_cleared timestamp
        std::string timestamp;
        line >> timestamp;
        log_message(LOG_INFO, "sort: spectra have been cleared.\n");
        sort_buffers = sort_errors = 0;
        sort_average_length = 0;
        gui_update_state();
        break; }
    case 205: // status_range
        log_message(LOG_INFO, "sort: range data have been updated.\n");
        break;
    case 206: { // status_user %s
	std::getline(line, remain);
        log_message(LOG_INFO, "sort: user routine identifier is: %s\n", remain.c_str());
        break; }
    case 207: { // status_cwd %s
        line.get();
        std::getline(line, sort_cwd);
        DBGV(sort_cwd);
        gui_update_state();
        break; }

    case 401: { // error_file %s -- could not dump to file
        getline(line, remain);
        log_message(LOG_ERR, "sort: could not dump. %s\n", remain.c_str());
        break; }
    case 402: { // error_file %s -- could not read gain/shift from file
        getline(line, remain);
        log_message(LOG_ERR, "sort: could not read gain/shift. %s\n", remain.c_str());
        break; }
    case 403: { // error_file %s -- could not read range data from file
        getline(line, remain);
        log_message(LOG_ERR, "sort: could not read range data. %s\n", remain.c_str());
        break; }
    case 404: { // error_file %s -- could not read telewin data from file
        getline(line, remain);
        log_message(LOG_ERR, "sort: could not read telewin data. %s\n", remain.c_str());
        break; }

    default:
        log_message(LOG_ERR, "sort: Unknown message '%s'\n", l.c_str());
    }
}

// ########################################################################

static void m_sort_disconnected(line_channel*, void*)
{
    log_message(LOG_ERR, "sort disconnected\n");

    delete lc_sort;
    lc_sort = 0;

    gui_update_state();
}

// ########################################################################

bool m_sort_connect(io_control& ioc)
{
    if( lc_sort )
        return true;


    lc_sort = line_connect(ioc, "127.0.0.1", 32010,
			   new line_cb(m_sort_disconnected),
			   new line_cb(m_sort_have_line));
    if( !lc_sort ) {
	commands->run("sort");
    #ifndef __APPLE__
        sleep(1);
    #endif // __APPLE__
	lc_sort = line_connect(ioc, "127.0.0.1", 32010,
			       new line_cb(m_sort_disconnected),
			       new line_cb(m_sort_have_line));
    }
    gui_update_state();

    return lc_sort != 0;
}

// ########################################################################

bool m_sort_connect(io_control& ioc, const char* host)
{
    if( lc_sort )
        return true;


    lc_sort = line_connect(ioc, host, 32010,
               new line_cb(m_sort_disconnected),
               new line_cb(m_sort_have_line));
    if( !lc_sort ) {
    commands->run("sort");
    #ifndef __APPLE__
        sleep(1);
    #endif // __APPLE__
    lc_sort = line_connect(ioc, host, 32010,
                   new line_cb(m_sort_disconnected),
                   new line_cb(m_sort_have_line));
    }
    gui_update_state();

    return lc_sort != 0;
}

// ########################################################################

bool m_sort_disconnect()
{
    delete lc_sort;
    lc_sort = 0;
    return true;
}

// ########################################################################

static bool m_sort_send(const char* cmd, const char* arg=0)
{
    if( !lc_sort )
        return false;

    line_sender ls(lc_sort);
    ls << cmd;
    if( arg )
        ls << ' ' << arg;
    ls << '\n';
    return true;
}

// ########################################################################

bool m_sort_quit()
{
    return m_sort_send("quit");
}

// ########################################################################

bool m_sort_clear()
{
    return m_sort_send("clear");
}

// ########################################################################

bool m_sort_dump()
{
    return m_sort_send("dump");
}

// ########################################################################

bool m_sort_is_connected()
{
    return lc_sort != 0;
}

// ########################################################################

bool m_sort_gainshift(const char* filename)
{
    return m_sort_send("gain", filename);
}

// ########################################################################

bool m_sort_telewin(const char* filename)
{
    return m_sort_send("telewin", filename);
}

// ########################################################################

bool m_sort_range(const char* filename)
{
    return m_sort_send("range", filename);
}

// ########################################################################

bool m_sort_status_gain()
{
    return m_sort_send("status_gain");
}

// ########################################################################

bool m_sort_status_telewin()
{
    return m_sort_send("status_telewin");
}

// ########################################################################

bool m_sort_status_user()
{
    return m_sort_send("user_id");
}

// ########################################################################

void m_sort_get_buffers(int& buffer_count, int& error_count, float& average_length)
{
    if( !lc_sort ) {
        buffer_count   = -1;
        error_count    = -1;
        average_length = -1;
    } else {
        buffer_count   = sort_buffers;
        error_count    = sort_errors;
        average_length = sort_average_length;
    }
}

// ########################################################################

bool m_sort_change_cwd(const char* dirname)
{
    return m_sort_send("change_cwd", dirname);
}

// ########################################################################

const char* m_sort_get_cwd()
{
    return sort_cwd.empty() ? 0 : sort_cwd.c_str();
}
