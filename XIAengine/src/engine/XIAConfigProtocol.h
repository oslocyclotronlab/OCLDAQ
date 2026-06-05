#ifndef XIA_CONFIG_PROTOCOL_H
#define XIA_CONFIG_PROTOCOL_H

#include <string>

namespace xia_config_protocol {

constexpr int DefaultConfigPort = 32011;
constexpr int NumberOfChannels = 16;

std::string encode_string(const std::string &value);
std::string decode_string(const std::string &value);

}

#endif // XIA_CONFIG_PROTOCOL_H
