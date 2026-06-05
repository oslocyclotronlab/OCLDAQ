#include "XIAConfigProtocol.h"

#include <cctype>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace xia_config_protocol {

std::string encode_string(const std::string &value)
{
    std::ostringstream out;
    out << std::hex << std::setfill('0');
    for (unsigned char c : value)
        out << std::setw(2) << static_cast<unsigned int>(c);
    return out.str();
}

std::string decode_string(const std::string &value)
{
    if (value.size() % 2 != 0)
        throw std::runtime_error("Encoded string has an odd number of characters");

    std::string out;
    out.reserve(value.size() / 2);
    for (size_t i = 0; i < value.size(); i += 2) {
        const unsigned char hi = static_cast<unsigned char>(value[i]);
        const unsigned char lo = static_cast<unsigned char>(value[i + 1]);
        if (!std::isxdigit(hi) || !std::isxdigit(lo))
            throw std::runtime_error("Encoded string contains a non-hex character");

        const auto byte = static_cast<char>(std::stoi(value.substr(i, 2), nullptr, 16));
        out.push_back(byte);
    }
    return out;
}

}
