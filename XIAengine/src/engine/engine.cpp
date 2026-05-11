
#include "engine_shm.h"
#include "net_control.h"
#include "utilities.h"
#include "run_command.h"

#include "WriteTerminal.h"
#include "XIAControl.h"
#include "functions.h"

#include "engine_daemon_context.h"
#include "engine_json_rpc.h"
#include "engine_state.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <cerrno>
#include <cstring>
#include <csignal>
#include <cstdlib>
#include <cstdio>
#include <sys/time.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>

#if _FILE_OFFSET_BITS != 64
#error must compile with _FILE_OFFSET_BITS == 64
#endif

static const int MAX_BUFFER_COUNT = 16384;

char leaveprog = 'n';
static int buffer_count = -1;
static float buffer_rate = 0;
static std::string output_filename;
static FILE *output_file = nullptr;
static unsigned int datalen_char = 1;
static timeval last_time = { 0, 0 };

static line_server *ls_engine = nullptr;
static line_server *ls_json = nullptr;

static WriteTerminal termWrite;
static XIAControl *xiacontr = nullptr;
command_list *commands = nullptr;

static EngineDaemonContext g_ctx;

#ifndef OFFLINE
#define OFFLINE false
#endif

// ########################################################################

void keyb_int(int sig_num)
{
    if (sig_num == SIGINT) {
        termWrite.Write("\n\nLeaving...\n");
        leaveprog = 'y';
    }
}

// ########################################################################

static void close_file()
{
    if (output_file) {
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

    if (output_filename.empty())
        return true;

    output_file = fopen(output_filename.c_str(), "ab");
    if (!output_file) {
        std::ostringstream out;
        out << "501 error_file Could not open '" << escape(output_filename) << "' for append.\n";
        ls_engine->send_all(out.str());
        return false;
    }
    const long fs = ftell(output_file);
    buffer_count = static_cast<int>(fs / static_cast<long>(datalen_char));
    char tmp[2048];
    snprintf(tmp, sizeof(tmp), "engine: file '%s' (%d buffers) was opened.\n", output_filename.c_str(), buffer_count);
    termWrite.Write(tmp);
    return true;
}

// ########################################################################

static void do_stop()
{
    xiacontr->XIA_end_run(output_file, output_filename.c_str());
    close_file();
    g_ctx.stopped = true;
    g_ctx.state = EngineState::Idle;
    last_time.tv_sec = last_time.tv_usec = 0;

    ls_engine->send_all("201 status_stopped\n");

    std::cout << "sleeping 1s to avoid confusing bobcat... " << std::flush;
    sleep(1);
    std::cout << "done" << std::endl;
}

// ########################################################################

static bool do_change_output_file(const std::string &fname)
{
    if (fname.empty() || fname == output_filename)
        return false;

    close_file();
    output_filename = fname;

    std::ostringstream out;
    out << "203 output_file " << escape(output_filename) << '\n';
    ls_engine->send_all(out.str());

    if (xiacontr)
        xiacontr->setFile(output_filename.c_str());

    if (g_ctx.state == EngineState::Run) {
        if (!open_file()) {
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
    if (dot == 0 || dot == std::string::npos) {
        std::cerr << "engine: dot not found in filename '" << output_filename
                  << "', will not change filename." << std::endl;
        return false;
    }

    std::string new_filename = output_filename;
    const std::string extension = "-big-";
    std::string::size_type ext = output_filename.find(extension);
    if (ext != std::string::npos && (ext + extension.size() + 3 == dot)) {
        std::string num = output_filename.substr(ext + extension.size(), 3);
        int n = 0;
        for (int i = 0; i < 3; ++i) {
            if (num[i] >= '0' && num[i] <= '9')
                n = 10 * n + (num[i] - '0');
        }
        if (n < 0 || n > 998)
            return false;
        n += 1;
        new_filename.replace(ext + extension.size(), 3, ioprintf("%03d", n));
    } else {
        const std::string i = extension + "000";
        new_filename.insert(dot, i);
    }
    if (file_exists(new_filename))
        return false;

    return do_change_output_file(new_filename);
}

// ########################################################################

static void command_quit(line_channel *lc, const std::string &, void *)
{
    if (g_ctx.state == EngineState::Run) {
        lc->send("401 error_state Can only quit if stopped.\n");
        return;
    }
    close_file();
    leaveprog = 'y';
}

// ########################################################################

static void command_stop(line_channel *lc, const std::string &, void *)
{
    if (g_ctx.state != EngineState::Run) {
        return lc->send("402 error_state Already stopped.\n");
    }
    do_stop();
}

// ########################################################################

static void broadcast_buffer_count()
{
    std::ostringstream out;
    out << "101 buffer_count " << buffer_count << ' ' << buffer_rate << '\n';
    ls_engine->send_all(out.str());
}

// ########################################################################

static void command_start(line_channel *lc, const std::string &, void *)
{
    if (g_ctx.state != EngineState::Idle) {
        line_sender ls(lc);
        ls << "403 error_state start requires idle state (current: " << engine_state_cstr(g_ctx.state) << ")\n";
        return;
    }
    if (!open_file())
        return;
    xiacontr->XIA_start_run();
    if (!xiacontr->XIA_check_status()) {
        close_file();
        lc->send("502 error_vme Could not connect to VME - eventbuilder stopped?.\n");
        return;
    }

    g_ctx.stopped = false;
    g_ctx.state = EngineState::Run;

    ls_engine->send_all("202 status_started\n");
    broadcast_buffer_count();
}

// ########################################################################

static void command_output_get_dir(line_channel *lc, const std::string &, void *)
{
    char cwd[1024];
    if (!getcwd(cwd, sizeof(cwd))) {
        line_sender ls(lc);
        if (errno == ENOENT) {
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

static void command_status(line_channel *lc, const std::string &, void *)
{
    lc->send(g_ctx.state == EngineState::Run ? "202 status_started\n" : "201 status_stopped\n");
    {
        line_sender ls(lc);
        ls << "206 engine_state " << engine_state_cstr(g_ctx.state) << '\n';
    }
    if (!output_filename.empty()) {
        line_sender ls(lc);
        ls << "203 output_file " << escape(output_filename) << '\n';
    } else {
        lc->send("204 output_none\n");
    }
    command_output_get_dir(lc, "status", nullptr);
    if (g_ctx.state == EngineState::Run) {
        line_sender ls(lc);
        ls << "101 buffer_count " << buffer_count << '\n';
    }
}

// ########################################################################

static void command_output_none(line_channel *lc, const std::string &, void *)
{
    if (output_filename.empty()) {
        lc->send("404 error_cmd 'none' output already selected.\n");
        return;
    }

    close_file();
    output_filename = "";
    if (xiacontr)
        xiacontr->setFile(output_filename.c_str());

    ls_engine->send_all("204 output_none\n");
}

// ########################################################################

static void command_output_file(line_channel *lc, const std::string &line, void *)
{
    const std::string fname = line.substr(12);
    if (!do_change_output_file(fname)) {
        line_sender ls(lc);
        ls << "405 error_file Cannot select file '" << escape(fname) << "'.\n";
    }
}

// ########################################################################

static void command_output_dir(line_channel *lc, const std::string &line, void *)
{
    if (g_ctx.state == EngineState::Run) {
        line_sender ls(lc);
        ls << "406 error_dir Cannot change directory while started.\n";
        return;
    }
    const std::string dirname = line.substr(11);
    if (chdir(dirname.c_str()) != 0) {
        line_sender ls(lc);
        ls << "406 error_dir Cannot change to directory '" << escape(dirname) << "'.\n";
    } else {
        std::ostringstream out;
        out << "205 output_dir " << dirname << '\n';
        ls_engine->send_all(out.str());
    }
}

// ########################################################################

static void cb_connected(line_channel *lc, void *)
{
    termWrite.Write("engine: new client\n");
    command_status(lc, "status", nullptr);
}

static void cb_disconnected(line_channel *, void *)
{
    termWrite.Write("engine: client disconnected\n");
}

static void cb_json_connected(line_channel *, void *)
{
    termWrite.Write("engine: JSON RPC client connected\n");
}

static void cb_json_disconnected(line_channel *, void *)
{
    termWrite.Write("engine: JSON RPC client disconnected\n");
}

struct JsonRpcLineCb : line_callback {
    void run(line_channel *lc) override
    {
        engine_json_rpc_handle_line(lc, g_ctx, lc->get_line());
    }
};

// ########################################################################

static bool parse_slot_mapping(int argc, char **argv, unsigned short *PXIMapping)
{
    for (int k = 0; k < PRESET_MAX_MODULES; ++k)
        PXIMapping[k] = 0;

    if (argc <= 1) {
        try {
            auto mapping = ReadSlotMap();
            if (mapping.size() >= PRESET_MAX_MODULES) {
                std::cerr << "Too many PCI devices found, found " << mapping.size() << std::endl;
                return false;
            }
            int set = 0;
            for (auto &entry : mapping)
                PXIMapping[set++] = entry;
        } catch (std::exception &ex) {
            std::cerr << "Could not determine PLX slot mapping: " << ex.what() << std::endl;
            std::cerr << "Try with manual PXI slot mapping, e.g.:" << std::endl;
            std::cerr << argv[0] << " [--auto-boot] [--offline] 2 3 4 5" << std::endl;
            return false;
        }
    } else {
        for (int i = 1; i < argc; ++i)
            PXIMapping[i] = static_cast<unsigned short>(atoi(argv[i]));
    }
    return true;
}

static int run_main_loop(int argc, char **argv)
{
    commands = new command_list();
    if (commands->read("acq_master_commands.txt")) {
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
            "elog     = echo\n");
    }

    io_select ioc;

    static command_cb::command engine_commands[] = {
        {"quit", false, command_quit, nullptr},
        {"stop", false, command_stop, nullptr},
        {"start", false, command_start, nullptr},
        {"output_none", false, command_output_none, nullptr},
        {"output_file", true, command_output_file, nullptr},
        {"output_get_dir", false, command_output_get_dir, nullptr},
        {"output_dir", true, command_output_dir, nullptr},
        {"status", false, command_status, nullptr},
        {nullptr, false, nullptr, nullptr},
    };

    try {
        ls_engine = new line_server(ioc, 32009, "engine", new line_cb(cb_connected), new line_cb(cb_disconnected),
                                    new command_cb(engine_commands, "407 error_cmd"));
        ls_json = new line_server(ioc, 32010, "xiajson", new line_cb(cb_json_connected), new line_cb(cb_json_disconnected),
                                  new JsonRpcLineCb());
    } catch (const std::exception &ex) {
        std::cerr << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    unsigned int *buffer = engine_shm_attach(true);
    if (!buffer) {
        std::cerr << "engine: Failed to attach shared memory." << std::endl;
        return EXIT_FAILURE;
    }
    unsigned int *time_us = &buffer[ENGINE_TIME_US];
    unsigned int *time_s = &buffer[ENGINE_TIME_S];
    unsigned int *data = buffer + buffer[ENGINE_DATA_START];
    unsigned int *first_header = &buffer[ENGINE_FIRST_HEADER];
    const unsigned int datalen = buffer[ENGINE_DATA_SIZE];
    datalen_char = datalen * sizeof(int);

    while (leaveprog == 'n') {
        if (g_ctx.state == EngineState::Run) {
            if (xiacontr->XIA_check_buffer(static_cast<int>(datalen))) {
                *time_us = *time_s = 0;
                if (!xiacontr->XIA_fetch_buffer(data, static_cast<int>(datalen), first_header)) {
                    do_stop();
                } else {
                    timeval t{};
                    gettimeofday(&t, nullptr);
                    *time_us = t.tv_usec;
                    *time_s = t.tv_sec;

                    if (output_file) {
                        unsigned int w = fwrite(data, 1, datalen_char, output_file);
                        if (w != datalen_char) {
                            ls_engine->send_all("503 error_file Write error, closing file and stopping.\n");
                            do_stop();
                        }
                    }

                    if (last_time.tv_sec != 0 && last_time.tv_usec != 0) {
                        buffer_rate = static_cast<float>((t.tv_sec + 1e-6 * t.tv_usec)
                                                         - (last_time.tv_sec + 1e-6 * last_time.tv_usec));
                        if (buffer_rate > 0)
                            buffer_rate = 1.f / buffer_rate;
                        else
                            buffer_rate = 999999;
                    } else {
                        buffer_rate = 0;
                    }
                    last_time = t;

                    buffer_count += 1;
                    broadcast_buffer_count();
                    if (output_file && buffer_count == MAX_BUFFER_COUNT)
                        change_output_file();
                }
                continue;
            }

            if (!xiacontr->XIA_check_status())
                do_stop();
        }
        struct timeval timeout = {0, 250000};
        ioc.run(&timeout);
    }

    engine_shm_detach();
    delete commands;
    commands = nullptr;
    delete ls_engine;
    ls_engine = nullptr;
    delete ls_json;
    ls_json = nullptr;
    return 0;
}

// ########################################################################

static void try_auto_boot(bool offline)
{
    g_ctx.state = EngineState::Initializing;
    if (!xiacontr->XIA_boot_all(offline)) {
        xiacontr->shutdownPixie();
        g_ctx.xia_iface.reset();
        g_ctx.state = EngineState::Unconfigured;
        std::cerr << "Auto-boot failed; use JSON-RPC init_boot on port 32010." << std::endl;
        return;
    }
    const size_t nmod = static_cast<size_t>(xiacontr->GetNumMod());
    if (nmod < 1) {
        xiacontr->shutdownPixie();
        g_ctx.state = EngineState::Unconfigured;
        std::cerr << "No modules after boot." << std::endl;
        return;
    }
    g_ctx.xia_iface = std::make_unique<XIAInterfaceAPI2>(nmod);
    g_ctx.state = EngineState::Idle;
    g_ctx.stopped = true;
    std::cerr << "Auto-boot OK, " << nmod << " module(s), state=idle" << std::endl;
}

int main(int argc, char *argv[])
{
    const char *lock_file = "/tmp/XIAengine.lock";

    int fd = open(lock_file, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        std::cerr << "Could not open lock file: " << lock_file << " (" << std::strerror(errno) << ")\n";
        return EXIT_FAILURE;
    }
    if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
        if (errno == EWOULDBLOCK)
            std::cerr << "Another instance of XIAengine is already running.\n";
        else
            std::cerr << "Failed to lock file: " << lock_file << " (" << std::strerror(errno) << ")\n";
        close(fd);
        return EXIT_FAILURE;
    }

    ftruncate(fd, 0);
    {
        std::string pidStr = std::to_string(getpid()) + "\n";
        write(fd, pidStr.c_str(), pidStr.size());
    }

    bool auto_boot = false;
    bool offline_boot = OFFLINE;
    std::vector<char *> slot_argv;
    slot_argv.push_back(argv[0]);
    for (int i = 1; i < argc; ++i) {
        std::string a(argv[i]);
        if (a == "--auto-boot")
            auto_boot = true;
        else if (a == "--offline")
            offline_boot = true;
        else
            slot_argv.push_back(argv[i]);
    }
    const int slot_argc = static_cast<int>(slot_argv.size());

    unsigned short PXIMapping[PRESET_MAX_MODULES];
    if (!parse_slot_mapping(slot_argc, slot_argv.data(), PXIMapping))
        return EXIT_FAILURE;

    signal(SIGINT, keyb_int);
    signal(SIGPIPE, SIG_IGN);
    usleep(10);

    xiacontr = new XIAControl(&termWrite, PXIMapping);
    g_ctx.xiacontr = xiacontr;
    g_ctx.state = EngineState::Unconfigured;
    g_ctx.stopped = true;

    if (auto_boot)
        try_auto_boot(offline_boot);

    const int rc = run_main_loop(argc, argv);

    g_ctx.xia_iface.reset();
    delete xiacontr;
    xiacontr = nullptr;
    g_ctx.xiacontr = nullptr;

    flock(fd, LOCK_UN);
    close(fd);
    return rc;
}
