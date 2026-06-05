#include "XIAConfigServer.h"

#include "XIAConfigProtocol.h"
#include "xiainterface.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

template <typename T>
T parse_field(std::istringstream &input, const char *name)
{
    T value{};
    if (!(input >> value))
        throw std::runtime_error(std::string("Missing or invalid field: ") + name);
    return value;
}

std::string parse_encoded_field(std::istringstream &input, const char *name)
{
    const auto encoded = parse_field<std::string>(input, name);
    return xia_config_protocol::decode_string(encoded);
}

void require_end(std::istringstream &input)
{
    std::string extra;
    if (input >> extra)
        throw std::runtime_error("Too many arguments");
}

void send_ok(line_channel *lc)
{
    line_sender ls(lc);
    ls << "200 config_ok\n";
}

}

XIAConfigServer::XIAConfigServer(io_control &ioc, int port, XIAInterface &interface,
                                 CanConfigureCallback can_configure)
    : interface_(interface)
    , can_configure_(std::move(can_configure))
    , commands_{
          Command{"config_hello", false, command_hello, this},
          Command{"config_num_modules", false, command_num_modules, this},
          Command{"config_module_info", true, command_module_info, this},
          Command{"config_chan_limits", true, command_chan_limits, this},
          Command{"config_get_chan_param", true, command_get_chan_param, this},
          Command{"config_set_chan_param", true, command_set_chan_param, this},
          Command{"config_mod_limits", true, command_mod_limits, this},
          Command{"config_get_mod_param", true, command_get_mod_param, this},
          Command{"config_set_mod_param", true, command_set_mod_param, this},
          Command{"config_measure_blcut", true, command_measure_blcut, this},
          Command{"config_measure_baseline", true, command_measure_baseline, this},
          Command{"config_copy_dsp", true, command_copy_dsp, this},
          Command{"config_write_settings", true, command_write_settings, this},
          Command{nullptr, false, nullptr, nullptr},
      }
    , server_(std::make_unique<line_server>(
          ioc,
          port,
          "xia_config",
          new line_cb(cb_connected, this),
          new line_cb(cb_disconnected, this),
          new command_cb(commands_.data(), "507 config_error")))
{
}

XIAConfigServer::~XIAConfigServer() = default;

XIAConfigServer &XIAConfigServer::from_user_data(void *user_data)
{
    if (!user_data)
        throw std::runtime_error("Missing XIAConfigServer user data");
    return *static_cast<XIAConfigServer *>(user_data);
}

void XIAConfigServer::send_error(line_channel *lc, const std::string &message)
{
    line_sender ls(lc);
    ls << "500 config_error " << xia_config_protocol::encode_string(message) << '\n';
}

void XIAConfigServer::require_can_configure() const
{
    if (can_configure_ && !can_configure_())
        throw std::runtime_error("Configuration is locked while a run is active");
}

void XIAConfigServer::cb_connected(line_channel *, void *)
{
    std::cout << "xia_config: new client" << std::endl;
}

void XIAConfigServer::cb_disconnected(line_channel *, void *)
{
    std::cout << "xia_config: client disconnected" << std::endl;
}

void XIAConfigServer::command_hello(line_channel *lc, const std::string &, void *)
{
    line_sender ls(lc);
    ls << "200 config_ok 1\n";
}

void XIAConfigServer::command_num_modules(line_channel *lc, const std::string &, void *user_data)
{
    try {
        auto &server = from_user_data(user_data);
        line_sender ls(lc);
        ls << "210 config_num_modules " << server.interface_.GetNumModules() << '\n';
    } catch (const std::exception &ex) {
        send_error(lc, ex.what());
    }
}

void XIAConfigServer::command_module_info(line_channel *lc, const std::string &line, void *user_data)
{
    try {
        auto &server = from_user_data(user_data);
        std::istringstream input(line);
        std::string command;
        input >> command;
        const auto module = parse_field<size_t>(input, "module");
        require_end(input);

        const auto info = server.interface_.GetModuleInfo(module);
        line_sender ls(lc);
        ls << "211 config_module_info "
           << info.revision << ' '
           << info.adc_bits << ' '
           << info.adc_msps << ' '
           << info.serial_number << '\n';
    } catch (const std::exception &ex) {
        send_error(lc, ex.what());
    }
}

void XIAConfigServer::command_chan_limits(line_channel *lc, const std::string &line, void *user_data)
{
    try {
        auto &server = from_user_data(user_data);
        std::istringstream input(line);
        std::string command;
        input >> command;
        const auto module = parse_field<size_t>(input, "module");
        const auto channel = parse_field<size_t>(input, "channel");
        const auto name = parse_encoded_field(input, "parameter");
        require_end(input);

        const auto limits = server.interface_.GetChnLimits(module, channel, name.c_str());
        line_sender ls(lc);
        ls << "212 config_chan_limits " << limits.first << ' ' << limits.second << '\n';
    } catch (const std::exception &ex) {
        send_error(lc, ex.what());
    }
}

void XIAConfigServer::command_get_chan_param(line_channel *lc, const std::string &line, void *user_data)
{
    try {
        auto &server = from_user_data(user_data);
        std::istringstream input(line);
        std::string command;
        input >> command;
        const auto module = parse_field<size_t>(input, "module");
        const auto channel = parse_field<size_t>(input, "channel");
        const auto name = parse_encoded_field(input, "parameter");
        require_end(input);

        const auto value = server.interface_.GetChnParam(module, channel, name.c_str());
        line_sender ls(lc);
        ls << "213 config_chan_param " << value << '\n';
    } catch (const std::exception &ex) {
        send_error(lc, ex.what());
    }
}

void XIAConfigServer::command_set_chan_param(line_channel *lc, const std::string &line, void *user_data)
{
    try {
        auto &server = from_user_data(user_data);
        std::istringstream input(line);
        std::string command;
        input >> command;
        const auto module = parse_field<size_t>(input, "module");
        const auto channel = parse_field<size_t>(input, "channel");
        const auto name = parse_encoded_field(input, "parameter");
        const auto value = parse_field<XIAInterface::ChanPar_t>(input, "value");
        require_end(input);
        server.require_can_configure();

        server.interface_.SetChnParam(module, channel, name.c_str(), value);
        send_ok(lc);
    } catch (const std::exception &ex) {
        send_error(lc, ex.what());
    }
}

void XIAConfigServer::command_mod_limits(line_channel *lc, const std::string &line, void *user_data)
{
    try {
        auto &server = from_user_data(user_data);
        std::istringstream input(line);
        std::string command;
        input >> command;
        const auto module = parse_field<size_t>(input, "module");
        const auto name = parse_encoded_field(input, "parameter");
        require_end(input);

        const auto limits = server.interface_.GetModLimits(module, name.c_str());
        line_sender ls(lc);
        ls << "214 config_mod_limits " << limits.first << ' ' << limits.second << '\n';
    } catch (const std::exception &ex) {
        send_error(lc, ex.what());
    }
}

void XIAConfigServer::command_get_mod_param(line_channel *lc, const std::string &line, void *user_data)
{
    try {
        auto &server = from_user_data(user_data);
        std::istringstream input(line);
        std::string command;
        input >> command;
        const auto module = parse_field<size_t>(input, "module");
        const auto name = parse_encoded_field(input, "parameter");
        require_end(input);

        const auto value = server.interface_.GetModParam(module, name.c_str());
        line_sender ls(lc);
        ls << "215 config_mod_param " << value << '\n';
    } catch (const std::exception &ex) {
        send_error(lc, ex.what());
    }
}

void XIAConfigServer::command_set_mod_param(line_channel *lc, const std::string &line, void *user_data)
{
    try {
        auto &server = from_user_data(user_data);
        std::istringstream input(line);
        std::string command;
        input >> command;
        const auto module = parse_field<size_t>(input, "module");
        const auto name = parse_encoded_field(input, "parameter");
        const auto value = parse_field<XIAInterface::ModPar_t>(input, "value");
        require_end(input);
        server.require_can_configure();

        server.interface_.SetModParam(module, name.c_str(), value);
        send_ok(lc);
    } catch (const std::exception &ex) {
        send_error(lc, ex.what());
    }
}

void XIAConfigServer::command_measure_blcut(line_channel *lc, const std::string &line, void *user_data)
{
    try {
        auto &server = from_user_data(user_data);
        std::istringstream input(line);
        std::string command;
        input >> command;
        const auto module = parse_field<unsigned short>(input, "module");
        const auto channel = parse_field<unsigned short>(input, "channel");
        require_end(input);
        server.require_can_configure();

        const auto value = server.interface_.MeasureBLCut(module, channel);
        line_sender ls(lc);
        ls << "216 config_blcut " << value << '\n';
    } catch (const std::exception &ex) {
        send_error(lc, ex.what());
    }
}

void XIAConfigServer::command_measure_baseline(line_channel *lc, const std::string &line, void *user_data)
{
    try {
        auto &server = from_user_data(user_data);
        std::istringstream input(line);
        std::string command;
        input >> command;
        const auto module = parse_field<unsigned short>(input, "module");
        require_end(input);
        server.require_can_configure();

        server.interface_.MeasureBaseline(module);
        send_ok(lc);
    } catch (const std::exception &ex) {
        send_error(lc, ex.what());
    }
}

void XIAConfigServer::command_copy_dsp(line_channel *lc, const std::string &line, void *user_data)
{
    try {
        auto &server = from_user_data(user_data);
        std::istringstream input(line);
        std::string command;
        input >> command;
        const auto bitmap = parse_field<unsigned short>(input, "bitmap");
        const auto source_module = parse_field<unsigned short>(input, "source_module");
        const auto source_channel = parse_field<unsigned short>(input, "source_channel");
        const auto destination_mask = parse_field<std::string>(input, "destination_mask");
        require_end(input);
        server.require_can_configure();

        std::vector<unsigned short> mask(
            xia_config_protocol::NumberOfChannels * server.interface_.GetNumModules(), 0);
        if (destination_mask.size() != mask.size())
            throw std::runtime_error("Destination mask has wrong length");

        for (size_t i = 0; i < destination_mask.size(); ++i) {
            if (destination_mask[i] == '0')
                mask[i] = 0;
            else if (destination_mask[i] == '1')
                mask[i] = 1;
            else
                throw std::runtime_error("Destination mask may only contain 0 and 1");
        }

        const auto retval = server.interface_.CopyDSPParameters(
            bitmap, source_module, source_channel, mask.data());
        line_sender ls(lc);
        ls << "217 config_copy_dsp " << retval << '\n';
    } catch (const std::exception &ex) {
        send_error(lc, ex.what());
    }
}

void XIAConfigServer::command_write_settings(line_channel *lc, const std::string &line, void *user_data)
{
    try {
        auto &server = from_user_data(user_data);
        std::istringstream input(line);
        std::string command;
        input >> command;
        const auto filename = parse_encoded_field(input, "filename");
        require_end(input);
        server.require_can_configure();

        const auto ok = server.interface_.WriteSettings(filename.c_str());
        line_sender ls(lc);
        ls << "218 config_write_settings " << (ok ? 1 : 0) << '\n';
    } catch (const std::exception &ex) {
        send_error(lc, ex.what());
    }
}
