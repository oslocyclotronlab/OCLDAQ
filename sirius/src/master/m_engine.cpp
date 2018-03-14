
#include "m_engine.h"

#include "acq_gui.h"
#include "acq_log.h"
#include "net_control.h"
#include "run_command.h"

#include <iostream>
#include <string>
#include <sstream>

#include <stdlib.h>
#include <unistd.h>

#if TEST_STUFF
#endif // TEST_STUFF

#define NDEBUG
#include "debug.h"

extern command_list* commands;

static line_channel* lc_engine = 0;
static bool engine_started = false;
static std::string engine_output;
static std::string engine_output_dir;
static int engine_buffer_count;
static float engine_buffer_rate;

// ########################################################################

static void m_engine_have_line(line_channel*, void*)
{
    const std::string l = lc_engine->get_line();
    DBGV(l);

    std::istringstream line(l.c_str());
    int message_code = -1;
    std::string message_name = "?";
    line >> message_code >> message_name;

    switch( message_code ) {
    case 101: // buffer_count %d(count) %f(rate)
        engine_buffer_count = -1;
        engine_buffer_rate  = -2;
        line >> engine_buffer_count >> engine_buffer_rate;
        gui_update_state();
        break;

    case 201: // status_stopped
        engine_started = false;
        gui_update_state();
        break;
    case 202: // status_started
        engine_started = true;
        engine_buffer_count = 0;
        gui_update_state();
        break;
    case 203: { // output_file %s
        line >> engine_output;
        gui_update_state();
        break; }
    case 204: // output_none
        engine_output = "";
        gui_update_state();
        break;
    case 205: // output_dir
        line >> engine_output_dir;
        gui_update_state();
        break;

    case 401:   // cannot quit
    case 402:   // already stopped
    case 403:   // already started
    case 404:   // must stop to change output to file
    case 405:   // 'none' already selected
    case 407:   // unknown command

    case 501:   // file could not be opened
    case 503:   // problem writing file
    case 502: { // vme problem
        std::string message = "?";
        getline(line, message);
        log_message(LOG_ERR, "engine: ERROR %s\n", message.c_str());
        break; }
    
    case 504: { // XIA problem
    	std::string message = "?";
    	getline(line, message);
    	log_message(LOG_ERR, "engine: ERROR %s\n", message.c_str());
    	break; }
    default:
        log_message(LOG_ERR, "engine: Unknown message '%s'\n", l.c_str());
    }
}

// ########################################################################

static void m_engine_disconnected(line_channel*, void*)
{
    log_message(LOG_ERR, "engine disconnected\n");

    delete lc_engine;
    lc_engine = 0;

    gui_update_state();
}

// ########################################################################

bool m_engine_connect(io_control& ioc)
{
    if( lc_engine )
        return true;

    lc_engine = line_connect(ioc, "127.0.0.1", 32009,
			     new line_cb(m_engine_disconnected),
			     new line_cb(m_engine_have_line));
    if( !lc_engine ) {
	commands->run("engine");
    #ifndef __APPLE__
        sleep(1);
    #endif // __APPLE__
	lc_engine = line_connect(ioc, "127.0.0.1", 32009,
				 new line_cb(m_engine_disconnected),
				 new line_cb(m_engine_have_line));
    }
    gui_update_state();
    return lc_engine != 0;
}

// ########################################################################

bool m_engine_connect(io_control& ioc, const char *host)
{
    if( lc_engine )
        return true;

    lc_engine = line_connect(ioc, host, 32009,
                 new line_cb(m_engine_disconnected),
                 new line_cb(m_engine_have_line));
    if( !lc_engine ) {
    commands->run("engine");
    #ifndef __APPLE__
        sleep(1);
    #endif // __APPLE__
    lc_engine = line_connect(ioc, host, 32009,
                 new line_cb(m_engine_disconnected),
                 new line_cb(m_engine_have_line));
    }
    gui_update_state();
    return lc_engine != 0;
}

// ########################################################################

bool m_engine_disconnect()
{
    delete lc_engine;
    lc_engine = 0;

    gui_update_state();

    return true;
}


// ########################################################################

static bool m_engine_send(const char* cmd, const char* arg=0)
{
    if( !lc_engine )
        return false;

    line_sender ls(lc_engine);
    ls << cmd;
    if( arg )
        ls << ' ' << arg;
    ls << '\n';
    return true;
}

// ########################################################################

bool m_engine_quit()
{
    return m_engine_send("quit");
}

// ########################################################################

bool m_engine_stop()
{
    return m_engine_send("stop");
}

// ########################################################################

bool m_engine_start()
{
    return m_engine_send("start");
}

// ########################################################################

bool m_engine_status()
{
    return m_engine_send("status");
}

// ########################################################################

bool m_engine_reload()
{
    return m_engine_send("reload");
}

// ########################################################################

bool m_engine_output_none()
{
    return m_engine_send("output_none");
}

// ########################################################################

bool m_engine_output_filename(const char* filename)
{
    return m_engine_send("output_file", filename);
}

// ########################################################################

bool m_engine_output_dir(const char* dirname)
{
    return m_engine_send("output_dir", dirname);
}

// ########################################################################

bool m_engine_is_started()
{
    return lc_engine!=0 && engine_started;
}

// ########################################################################

bool m_engine_is_connected()
{
    return lc_engine != 0;
}

// ########################################################################

const char* m_engine_get_output()
{
    return engine_output.empty() ? 0 : engine_output.c_str();
}

// ########################################################################

const char* m_engine_get_output_dir()
{
    return engine_output_dir.empty() ? 0 : engine_output_dir.c_str();
}

// ########################################################################

int m_engine_get_buffercount()
{
    return engine_buffer_count;
}
// ########################################################################

float m_engine_get_buffer_rate()
{
    return engine_buffer_rate;
}
