#include "xiainterface_remote.h"

#include "XIAConfigProtocol.h"
#include "net_control.h"

#include <chrono>
#include <deque>
#include <sstream>
#include <stdexcept>

namespace {

constexpr auto RequestTimeout = std::chrono::seconds(60);

class ClientLineCallback : public line_callback {
public:
    explicit ClientLineCallback(std::deque<std::string> &lines)
        : lines_(lines)
    {
    }

    void run(line_channel *lc) override
    {
        lines_.push_back(lc->get_line());
    }

private:
    std::deque<std::string> &lines_;
};

class ClientDisconnectCallback : public line_callback {
public:
    explicit ClientDisconnectCallback(bool &disconnected)
        : disconnected_(disconnected)
    {
    }

    void run(line_channel *) override
    {
        disconnected_ = true;
    }

private:
    bool &disconnected_;
};

template <typename T>
T parse_field(std::istringstream &input, const char *name)
{
    T value{};
    if (!(input >> value))
        throw std::runtime_error(std::string("Missing or invalid response field: ") + name);
    return value;
}

void require_status(const std::string &line, const std::string &code, const std::string &label)
{
    std::istringstream input(line);
    const auto actual_code = parse_field<std::string>(input, "status_code");
    const auto actual_label = parse_field<std::string>(input, "status_label");
    if (actual_code != code || actual_label != label)
        throw std::runtime_error("Unexpected config response: " + line);
}

void throw_if_error_response(const std::string &line)
{
    std::istringstream input(line);
    std::string code;
    std::string label;
    input >> code >> label;
    if (code != "500" && code != "507")
        return;

    if (code == "507")
        throw std::runtime_error(line);

    std::string encoded;
    input >> encoded;
    if (encoded.empty())
        throw std::runtime_error(line);

    throw std::runtime_error(xia_config_protocol::decode_string(encoded));
}

}

class XIAInterfaceRemote::ConfigClient {
public:
    ConfigClient(const std::string &host, int port)
    {
        channel_ = line_connect(ioc_, host.c_str(), port,
                                new ClientDisconnectCallback(disconnected_),
                                new ClientLineCallback(lines_));
        if (!channel_)
            throw std::runtime_error("Could not connect to XIA config server at " + host + ":" + std::to_string(port));
    }

    ~ConfigClient()
    {
        delete channel_;
    }

    std::string request(const std::string &line)
    {
        if (disconnected_)
            throw std::runtime_error("XIA config server is disconnected");

        channel_->send(line + "\n");

        const auto deadline = std::chrono::steady_clock::now() + RequestTimeout;
        while (lines_.empty()) {
            if (disconnected_)
                throw std::runtime_error("XIA config server disconnected while waiting for response");

            if (std::chrono::steady_clock::now() >= deadline)
                throw std::runtime_error("Timed out waiting for XIA config response");

            timeval timeout{0, 100000};
            ioc_.run(&timeout);
        }

        auto response = lines_.front();
        lines_.pop_front();
        throw_if_error_response(response);
        return response;
    }

private:
    io_select ioc_;
    line_channel *channel_ = nullptr;
    std::deque<std::string> lines_;
    bool disconnected_ = false;
};

size_t XIAInterfaceRemote::ProbeNumModules(const std::string &host, int port)
{
    ConfigClient client(host, port);
    const auto line = client.request("config_num_modules");
    require_status(line, "210", "config_num_modules");

    std::istringstream input(line);
    std::string code;
    std::string label;
    input >> code >> label;
    return parse_field<size_t>(input, "num_modules");
}

XIAInterfaceRemote::XIAInterfaceRemote(const std::string &host, int port, size_t num_modules)
    : XIAInterface(num_modules)
    , client_(std::make_unique<ConfigClient>(host, port))
    , moduleInfo_(num_modules)
{
    require_status(client_->request("config_hello"), "200", "config_ok");

    for (size_t module = 0; module < num_modules; ++module) {
        std::ostringstream request;
        request << "config_module_info " << module;
        const auto line = client_->request(request.str());
        require_status(line, "211", "config_module_info");

        std::istringstream input(line);
        std::string code;
        std::string label;
        input >> code >> label;
        moduleInfo_[module].revision = parse_field<unsigned short>(input, "revision");
        moduleInfo_[module].adc_bits = parse_field<unsigned short>(input, "adc_bits");
        moduleInfo_[module].adc_msps = parse_field<unsigned short>(input, "adc_msps");
        moduleInfo_[module].serial_number = parse_field<unsigned int>(input, "serial_number");
    }
}

XIAInterfaceRemote::~XIAInterfaceRemote() = default;

XIAInterface::ModuleInfo_t XIAInterfaceRemote::GetModuleInfo(const size_t &moduleID) const
{
    if (moduleID >= moduleInfo_.size())
        throw std::runtime_error("Module index is out of range");
    return moduleInfo_[moduleID];
}

XIAInterface::ChanLim_t XIAInterfaceRemote::GetChnLimits(const size_t &module, const size_t &channel,
                                                         const char *ChanParName)
{
    std::ostringstream request;
    request << "config_chan_limits " << module << ' ' << channel << ' '
            << xia_config_protocol::encode_string(ChanParName);
    const auto line = client_->request(request.str());
    require_status(line, "212", "config_chan_limits");

    std::istringstream input(line);
    std::string code;
    std::string label;
    input >> code >> label;
    const auto min = parse_field<ChanPar_t>(input, "min");
    const auto max = parse_field<ChanPar_t>(input, "max");
    return {min, max};
}

XIAInterface::ChanPar_t XIAInterfaceRemote::GetChnParam(const size_t &module, const size_t &channel,
                                                        const char *ChanParName)
{
    std::ostringstream request;
    request << "config_get_chan_param " << module << ' ' << channel << ' '
            << xia_config_protocol::encode_string(ChanParName);
    const auto line = client_->request(request.str());
    require_status(line, "213", "config_chan_param");

    std::istringstream input(line);
    std::string code;
    std::string label;
    input >> code >> label;
    return parse_field<ChanPar_t>(input, "value");
}

void XIAInterfaceRemote::SetChnParam(const size_t &module, const size_t &channel, const char *ChanParName,
                                     const XIAInterface::ChanPar_t &parameter)
{
    std::ostringstream request;
    request << "config_set_chan_param " << module << ' ' << channel << ' '
            << xia_config_protocol::encode_string(ChanParName) << ' ' << parameter;
    require_status(client_->request(request.str()), "200", "config_ok");
}

XIAInterface::ModLim_t XIAInterfaceRemote::GetModLimits(const size_t &module, const char *ModParName)
{
    std::ostringstream request;
    request << "config_mod_limits " << module << ' ' << xia_config_protocol::encode_string(ModParName);
    const auto line = client_->request(request.str());
    require_status(line, "214", "config_mod_limits");

    std::istringstream input(line);
    std::string code;
    std::string label;
    input >> code >> label;
    const auto min = parse_field<ModPar_t>(input, "min");
    const auto max = parse_field<ModPar_t>(input, "max");
    return {min, max};
}

XIAInterface::ModPar_t XIAInterfaceRemote::GetModParam(const size_t &module, const char *ModParName)
{
    std::ostringstream request;
    request << "config_get_mod_param " << module << ' ' << xia_config_protocol::encode_string(ModParName);
    const auto line = client_->request(request.str());
    require_status(line, "215", "config_mod_param");

    std::istringstream input(line);
    std::string code;
    std::string label;
    input >> code >> label;
    return parse_field<ModPar_t>(input, "value");
}

void XIAInterfaceRemote::SetModParam(const size_t &module, const char *ModParName,
                                     const XIAInterface::ModPar_t &parameter)
{
    std::ostringstream request;
    request << "config_set_mod_param " << module << ' '
            << xia_config_protocol::encode_string(ModParName) << ' ' << parameter;
    require_status(client_->request(request.str()), "200", "config_ok");
}

unsigned int XIAInterfaceRemote::MeasureBLCut(const unsigned short &module, const unsigned short &channel)
{
    std::ostringstream request;
    request << "config_measure_blcut " << module << ' ' << channel;
    const auto line = client_->request(request.str());
    require_status(line, "216", "config_blcut");

    std::istringstream input(line);
    std::string code;
    std::string label;
    input >> code >> label;
    return parse_field<unsigned int>(input, "blcut");
}

void XIAInterfaceRemote::MeasureBaseline(const unsigned short &module)
{
    std::ostringstream request;
    request << "config_measure_baseline " << module;
    require_status(client_->request(request.str()), "200", "config_ok");
}

int XIAInterfaceRemote::CopyDSPParameters(const unsigned short &BitMap, const unsigned short &sourceModule,
                                          const unsigned short &sourceChannel, unsigned short *DestinationMask)
{
    std::string mask;
    mask.reserve(xia_config_protocol::NumberOfChannels * GetNumModules());
    for (size_t i = 0; i < xia_config_protocol::NumberOfChannels * GetNumModules(); ++i)
        mask.push_back(DestinationMask[i] ? '1' : '0');

    std::ostringstream request;
    request << "config_copy_dsp " << BitMap << ' ' << sourceModule << ' ' << sourceChannel << ' ' << mask;
    const auto line = client_->request(request.str());
    require_status(line, "217", "config_copy_dsp");

    std::istringstream input(line);
    std::string code;
    std::string label;
    input >> code >> label;
    return parse_field<int>(input, "retval");
}

bool XIAInterfaceRemote::WriteSettings(const char *fname)
{
    std::ostringstream request;
    request << "config_write_settings " << xia_config_protocol::encode_string(fname);
    const auto line = client_->request(request.str());
    require_status(line, "218", "config_write_settings");

    std::istringstream input(line);
    std::string code;
    std::string label;
    input >> code >> label;
    return parse_field<int>(input, "success") != 0;
}
