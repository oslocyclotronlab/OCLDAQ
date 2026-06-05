#ifndef XIA_CONFIG_SERVER_H
#define XIA_CONFIG_SERVER_H

#include "net_control.h"

#include <array>
#include <functional>
#include <memory>
#include <string>

class XIAInterface;

class XIAConfigServer {
public:
    using CanConfigureCallback = std::function<bool()>;

    XIAConfigServer(io_control &ioc, int port, XIAInterface &interface,
                    CanConfigureCallback can_configure);
    ~XIAConfigServer();

private:
    using Command = command_cb::command;

    static void command_hello(line_channel *lc, const std::string &line, void *user_data);
    static void command_num_modules(line_channel *lc, const std::string &line, void *user_data);
    static void command_module_info(line_channel *lc, const std::string &line, void *user_data);
    static void command_chan_limits(line_channel *lc, const std::string &line, void *user_data);
    static void command_get_chan_param(line_channel *lc, const std::string &line, void *user_data);
    static void command_set_chan_param(line_channel *lc, const std::string &line, void *user_data);
    static void command_mod_limits(line_channel *lc, const std::string &line, void *user_data);
    static void command_get_mod_param(line_channel *lc, const std::string &line, void *user_data);
    static void command_set_mod_param(line_channel *lc, const std::string &line, void *user_data);
    static void command_measure_blcut(line_channel *lc, const std::string &line, void *user_data);
    static void command_measure_baseline(line_channel *lc, const std::string &line, void *user_data);
    static void command_copy_dsp(line_channel *lc, const std::string &line, void *user_data);
    static void command_write_settings(line_channel *lc, const std::string &line, void *user_data);

    static void cb_connected(line_channel *lc, void *user_data);
    static void cb_disconnected(line_channel *lc, void *user_data);

    static XIAConfigServer &from_user_data(void *user_data);
    static void send_error(line_channel *lc, const std::string &message);
    void require_can_configure() const;

    XIAInterface &interface_;
    CanConfigureCallback can_configure_;
    std::array<Command, 14> commands_;
    std::unique_ptr<line_server> server_;
};

#endif // XIA_CONFIG_SERVER_H
