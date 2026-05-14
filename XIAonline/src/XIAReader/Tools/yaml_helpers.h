//
// Created by Vetle Wegner Ingeberg on 27/04/2026.
//

#ifndef YAML_HELPERS_H
#define YAML_HELPERS_H

#include <magic_enum.hpp>
#include <yaml-cpp/yaml.h>

#include "DetectorTypes.h"
#include "SortTypes.h"
#include "XIA_CFD.h"

template <>
struct magic_enum::customize::enum_range<XIA::ADCSamplingFreq> {
    static constexpr int min = 0;
    static constexpr int max = 1000;
    // (max - min) must be less than UINT16_MAX.
};

namespace YAML {
    template<>
    struct convert<DetectorType> {
        static Node encode(const DetectorType &dtype){
            Node node;
            node.push_back(std::string(magic_enum::enum_name(dtype)));
            return node;
        }
        static bool decode(const Node &node, DetectorType &type){
            if ( !node.IsScalar() )
                return false;
            auto type_opt = magic_enum::enum_cast<DetectorType>(node.Scalar());
            if ( type_opt.has_value() ) {
                type = type_opt.value();
                return true;
            }
            return false;
        }
    };

    template<>
    struct convert<SortType> {
        static Node encode(const SortType &dtype){
            Node node;
            node.push_back(std::string(magic_enum::enum_name(dtype)));
            return node;
        }
        static bool decode(const Node &node, SortType &type){
            if ( !node.IsScalar() )
                return false;
            auto type_opt = magic_enum::enum_cast<SortType>(node.Scalar());
            if ( type_opt.has_value() ) {
                type = type_opt.value();
                return true;
            }
            return false;
        }
    };

    template<>
    struct convert<XIA::ADCSamplingFreq> {
        static Node encode(const XIA::ADCSamplingFreq &dtype){
            Node node;
            node.push_back(std::string(magic_enum::enum_name(dtype)));
            return node;
        }
        static bool decode(const Node &node, XIA::ADCSamplingFreq &type){
            if ( !node.IsScalar() )
                return false;
            auto freq = node.as<int>();
            switch ( freq ) {
                case 0:
                    type = XIA::f000MHz;
                    break;
                case 100:
                    type = XIA::f100MHz;
                    break;
                case 250:
                    type = XIA::f250MHz;
                    break;
                case 500:
                    type = XIA::f500MHz;
                    break;
                default:
                    return false;
            }
            return true;
        }
    };
}

#endif //YAML_HELPERS_H
