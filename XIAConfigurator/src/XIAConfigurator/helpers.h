//
// Created by Vetle Wegner Ingeberg on 02/12/2024.
//

#ifndef XIACONFIGURATOR_HELPERS_H
#define XIACONFIGURATOR_HELPERS_H

#include <string>
#include <QLoggingCategory>

#include "xiainterface.h"

QT_BEGIN_NAMESPACE
class QWidget;
class QHBoxLayout;
class QLabel;
QT_END_NAMESPACE


extern QLoggingCategory logger;

struct CSRmap_t {
    const unsigned short bit;
    const char *name;
};

template<typename T>
bool test_bit(const unsigned short& bit, T value) {
    return std::bitset<std::numeric_limits<T>::digits>(value).test(bit);
}

template<typename T>
T set_bit(const unsigned short &bit, const T &value, const bool &bit_status)
{
    auto value_bits = std::bitset<std::numeric_limits<T>::digits>(value);
    value_bits.set(bit, bit_status);
    return (T) value_bits.to_ulong();
}

template <typename I> std::string n2hexstr(I w, size_t hex_len = sizeof(I)<<1) {
    static const char* digits = "0123456789ABCDEF";
    std::string rc(hex_len,'0');
    for (size_t i=0, j=(hex_len-1)*4 ; i<hex_len; ++i,j-=4)
        rc[i] = digits[(w>>j) & 0x0f];
    return rc;
}

QHBoxLayout *getLayoutUnitless(QWidget *parent, const char *prefix, QWidget *widget);

template<typename B, typename T>
void get_numeric_limits(const size_t &module, const size_t &channel, T *widget, XIAInterface *interface, std::map<QWidget *, const char *> par_map)
{
    auto limits = interface->GetChnLimits(module, channel, par_map[widget]);
    qCDebug(logger) << "Parameter '" << par_map[widget] << "' limits: [" << *reinterpret_cast<B *>(&limits.first) << " " << *reinterpret_cast<B *>(&limits.second) << "]";
    widget->setMinimum(*reinterpret_cast<B *>(&limits.first));
    widget->setMaximum(*reinterpret_cast<B *>(&limits.second));
}

template<typename B, typename T>
void set_widget_numeric_value(const size_t &module, const size_t &channel, T *widget, XIAInterface *interface, std::map<QWidget *, const char *> par_map)
{
    auto value = interface->GetChnParam(module, channel, par_map[widget]);
    qCDebug(logger) << "Module " << module << " channel " << channel << ", setting parameter '" << par_map[widget] << "' from API, got " << *reinterpret_cast<B *>(&value);
    widget->setValue(*reinterpret_cast<B *>(&value));
}

template<typename T>
T read_channel_numeric_value(const size_t &module, const size_t &channel, XIAInterface *interface, const char *parName)
{
    auto value = interface->GetChnParam(module, channel, parName);
    return *reinterpret_cast<T *>(&value);
}

template<typename T>
T read_module_numeric_value(const size_t &module, XIAInterface *interface, const char *parName)
{
    auto value = interface->GetModParam(module, parName);
    return *reinterpret_cast<T *>(&value);
}

template<typename T>
T write_channel_value(const size_t &module, const size_t &channel, XIAInterface *interface, const char *parName, const T &value)
{
    T old_value = read_channel_numeric_value<T>(module, channel, interface, parName);
    if ( old_value != value ){
        interface->SetChnParam(module, channel, parName, *reinterpret_cast<const double *>(&value));
    }
    T new_value = read_channel_numeric_value<T>(module, channel, interface, parName);
    qCDebug(logger) << "Module " << module << " channel " << channel << ", updating parameter '" << parName << "' from API, old value=" << old_value << " new value=" << value << " value after upload=" << new_value;
    return new_value;
}

template<typename T>
T write_module_value(const size_t &module, XIAInterface *interface, const char *parName, const T &value)
{
    T old_value = read_module_numeric_value<T>(module, interface, parName);
    if ( old_value != value ){
        interface->SetModParam(module, parName, *reinterpret_cast<const unsigned int*>(&value));
    }
    T new_value = read_module_numeric_value<T>(module, interface, parName);
    qCDebug(logger) << "Module " << module << ", updating parameter '" << parName << "' from API, old value=" << old_value << " new value=" << value << " value after upload=" << new_value;
    return new_value;
}

#endif //XIACONFIGURATOR_HELPERS_H
