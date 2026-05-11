#include "xiainterface_remote.h"

#include <QJsonArray>
#include <QJsonDocument>

#include <stdexcept>

namespace {
constexpr int kCopyDestWords = 16 * 24; // NUMBER_OF_CHANNELS * PRESET_MAX_MODULES

QJsonObject read_one_response(QTcpSocket *sock, QByteArray &rxacc, int *req_id)
{
    while (sock->waitForReadyRead(120000)) {
        rxacc.append(sock->readAll());
        const int nl = rxacc.indexOf('\n');
        if (nl < 0)
            continue;
        QByteArray one = rxacc.left(nl);
        rxacc.remove(0, nl + 1);
        QJsonParseError pe{};
        QJsonDocument d = QJsonDocument::fromJson(one, &pe);
        if (!d.isObject())
            throw std::runtime_error("invalid JSON response");
        (void)req_id;
        return d.object();
    }
    throw std::runtime_error("timeout waiting for response");
}

QJsonObject do_rpc(QTcpSocket *sock, QByteArray &rxacc, int *req_id, const QJsonObject &req)
{
    QJsonObject r = req;
    r["id"] = ++(*req_id);
    QByteArray line = QJsonDocument(r).toJson(QJsonDocument::Compact);
    line += '\n';
    sock->write(line);
    sock->waitForBytesWritten(10000);
    return read_one_response(sock, rxacc, req_id);
}
} // namespace

std::unique_ptr<XIAInterfaceRemote> XIAInterfaceRemote::create(const QString &host, quint16 port,
                                                                 bool offline_boot, QString *error_message)
{
    auto sock = std::make_unique<QTcpSocket>();
    sock->connectToHost(host, port);
    if (!sock->waitForConnected(15000)) {
        if (error_message)
            *error_message = sock->errorString();
        return nullptr;
    }

    QByteArray rxacc;
    int sid = 0;

    try {
        QJsonObject hello = do_rpc(sock.get(), rxacc, &sid, QJsonObject{{"op", "hello"}});
        if (!hello.value("ok").toBool()) {
            if (error_message)
                *error_message = hello.value("error").toString();
            return nullptr;
        }
        QString st = hello.value("state").toString();
        int nmod = hello.value("nmod").toInt();
        if (st == QLatin1String("unconfigured")) {
            QJsonObject br = do_rpc(sock.get(), rxacc, &sid, QJsonObject{{"op", "init_boot"}, {"offline", offline_boot}});
            if (!br.value("ok").toBool()) {
                if (error_message)
                    *error_message = br.value("error").toString();
                return nullptr;
            }
            nmod = br.value("nmod").toInt();
        }
        if (nmod < 1) {
            if (error_message)
                *error_message = QStringLiteral("no modules");
            return nullptr;
        }

        auto iface = std::unique_ptr<XIAInterfaceRemote>(new XIAInterfaceRemote(static_cast<size_t>(nmod), std::move(sock)));
        iface->req_id_ = sid;
        return iface;
    } catch (const std::exception &ex) {
        if (error_message)
            *error_message = QString::fromUtf8(ex.what());
        return nullptr;
    }
}

XIAInterfaceRemote::XIAInterfaceRemote(size_t num_modules, std::unique_ptr<QTcpSocket> socket)
    : XIAInterface(num_modules)
    , sock_(std::move(socket))
{}

XIAInterfaceRemote::~XIAInterfaceRemote() = default;

QJsonObject XIAInterfaceRemote::rpc(const QJsonObject &req)
{
    QJsonObject r = req;
    r["id"] = ++req_id_;
    QByteArray line = QJsonDocument(r).toJson(QJsonDocument::Compact);
    line += '\n';
    if (!sock_)
        throw std::runtime_error("not connected");
    sock_->write(line);
    sock_->waitForBytesWritten(10000);
    while (sock_->waitForReadyRead(120000)) {
        rxbuf_.append(sock_->readAll());
        const int nl = rxbuf_.indexOf('\n');
        if (nl < 0)
            continue;
        QByteArray one = rxbuf_.left(nl);
        rxbuf_.remove(0, nl + 1);
        QJsonParseError pe{};
        QJsonDocument d = QJsonDocument::fromJson(one, &pe);
        if (!d.isObject())
            throw std::runtime_error("invalid JSON response");
        return d.object();
    }
    throw std::runtime_error("timeout waiting for response");
}

void XIAInterfaceRemote::ensure_ok(const QJsonObject &resp)
{
    if (!resp.value("ok").toBool()) {
        const QString err = resp.value("error").toString();
        throw std::runtime_error(err.isEmpty() ? "RPC error" : err.toStdString());
    }
}

XIAInterface::ModuleInfo_t XIAInterfaceRemote::GetModuleInfo(const size_t &moduleID) const
{
    QJsonObject req{{"op", "get_module_info"}, {"module", static_cast<qint64>(moduleID)}};
    auto *self = const_cast<XIAInterfaceRemote *>(this);
    QJsonObject o = self->rpc(req);
    self->ensure_ok(o);
    ModuleInfo_t m{};
    m.revision = static_cast<unsigned short>(o.value("revision").toInt());
    m.adc_bits = static_cast<unsigned short>(o.value("adc_bits").toInt());
    m.adc_msps = static_cast<unsigned short>(o.value("adc_msps").toInt());
    m.serial_number = static_cast<unsigned int>(o.value("serial_number").toVariant().toULongLong());
    return m;
}

XIAInterface::ChanLim_t XIAInterfaceRemote::GetChnLimits(const size_t &module, const size_t &channel,
                                                         const char *ChanParName)
{
    QJsonObject req{{"op", "get_chn_limits"},
                    {"module", static_cast<qint64>(module)},
                    {"channel", static_cast<qint64>(channel)},
                    {"name", QString::fromUtf8(ChanParName)}};
    QJsonObject o = rpc(req);
    ensure_ok(o);
    return {o.value("lo").toDouble(), o.value("hi").toDouble()};
}

XIAInterface::ChanPar_t XIAInterfaceRemote::GetChnParam(const size_t &module, const size_t &channel,
                                                        const char *ChanParName)
{
    QJsonObject req{{"op", "get_chn_param"},
                    {"module", static_cast<qint64>(module)},
                    {"channel", static_cast<qint64>(channel)},
                    {"name", QString::fromUtf8(ChanParName)}};
    QJsonObject o = rpc(req);
    ensure_ok(o);
    return o.value("value").toDouble();
}

void XIAInterfaceRemote::SetChnParam(const size_t &module, const size_t &channel, const char *ChanParName,
                                     const ChanPar_t &parameter)
{
    QJsonObject req{{"op", "set_chn_param"},
                    {"module", static_cast<qint64>(module)},
                    {"channel", static_cast<qint64>(channel)},
                    {"name", QString::fromUtf8(ChanParName)},
                    {"value", parameter}};
    QJsonObject o = rpc(req);
    ensure_ok(o);
}

XIAInterface::ModLim_t XIAInterfaceRemote::GetModLimits(const size_t &module, const char *ModParName)
{
    QJsonObject req{
        {"op", "get_mod_limits"}, {"module", static_cast<qint64>(module)}, {"name", QString::fromUtf8(ModParName)}};
    QJsonObject o = rpc(req);
    ensure_ok(o);
    return {static_cast<ModPar_t>(o.value("lo").toInt()), static_cast<ModPar_t>(o.value("hi").toInt())};
}

XIAInterface::ModPar_t XIAInterfaceRemote::GetModParam(const size_t &module, const char *ModParName)
{
    QJsonObject req{
        {"op", "get_mod_param"}, {"module", static_cast<qint64>(module)}, {"name", QString::fromUtf8(ModParName)}};
    QJsonObject o = rpc(req);
    ensure_ok(o);
    return static_cast<ModPar_t>(o.value("value").toVariant().toULongLong());
}

void XIAInterfaceRemote::SetModParam(const size_t &module, const char *ModParName, const ModPar_t &parameter)
{
    QJsonObject req{{"op", "set_mod_param"},
                    {"module", static_cast<qint64>(module)},
                    {"name", QString::fromUtf8(ModParName)},
                    {"value", static_cast<qint64>(parameter)}};
    QJsonObject o = rpc(req);
    ensure_ok(o);
}

unsigned int XIAInterfaceRemote::MeasureBLCut(const unsigned short &module, const unsigned short &channel)
{
    QJsonObject req{{"op", "measure_bl_cut"}, {"module", module}, {"channel", channel}};
    QJsonObject o = rpc(req);
    ensure_ok(o);
    return static_cast<unsigned int>(o.value("value").toVariant().toULongLong());
}

void XIAInterfaceRemote::MeasureBaseline(const unsigned short &module)
{
    QJsonObject req{{"op", "measure_baseline"}, {"module", module}};
    QJsonObject o = rpc(req);
    ensure_ok(o);
}

int XIAInterfaceRemote::CopyDSPParameters(const unsigned short &BitMap, const unsigned short &sourceModule,
                                          const unsigned short &sourceChannel, unsigned short *DestinationMask)
{
    QJsonArray arr;
    for (int i = 0; i < kCopyDestWords; ++i)
        arr.append(DestinationMask[i]);
    QJsonObject req{{"op", "copy_dsp_parameters"},
                    {"bitmap", BitMap},
                    {"source_module", sourceModule},
                    {"source_channel", sourceChannel},
                    {"dest_mask", arr}};
    QJsonObject o = rpc(req);
    ensure_ok(o);
    if (o.contains("dest_mask") && o.value("dest_mask").isArray()) {
        QJsonArray out = o.value("dest_mask").toArray();
        for (int i = 0; i < kCopyDestWords && i < out.size(); ++i)
            DestinationMask[i] = static_cast<unsigned short>(out.at(i).toInt());
    }
    return o.value("retval").toInt();
}

bool XIAInterfaceRemote::WriteSettings(const char *fname)
{
    QJsonObject req{{"op", "write_settings"}, {"path", QString::fromUtf8(fname)}};
    QJsonObject o = rpc(req);
    ensure_ok(o);
    return o.value("saved").toBool();
}
