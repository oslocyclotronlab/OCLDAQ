#ifndef XIAINTERFACE_REMOTE_H
#define XIAINTERFACE_REMOTE_H

#include "xiainterface.h"

#include <memory>

#include <QByteArray>
#include <QJsonObject>
#include <QString>
#include <QTcpSocket>

/** TCP client for XIAengine JSON-RPC (port 32010). */
class XIAInterfaceRemote : public XIAInterface
{
public:
    /** Connect, optional init_boot if daemon is unconfigured. Returns nullptr on failure. */
    static std::unique_ptr<XIAInterfaceRemote> create(const QString &host, quint16 port, bool offline_boot,
                                                      QString *error_message = nullptr);

    ~XIAInterfaceRemote() override;

    ModuleInfo_t GetModuleInfo(const size_t &moduleID) const override;

    ChanLim_t GetChnLimits(const size_t &module, const size_t &channel, const char *ChanParName) override;
    ChanPar_t GetChnParam(const size_t &module, const size_t &channel, const char *ChanParName) override;
    void SetChnParam(const size_t &module, const size_t &channel, const char *ChanParName,
                     const ChanPar_t &parameter) override;

    ModLim_t GetModLimits(const size_t &module, const char *ModParName) override;
    ModPar_t GetModParam(const size_t &module, const char *ModParName) override;
    void SetModParam(const size_t &module, const char *ModParName, const ModPar_t &parameter) override;

    unsigned int MeasureBLCut(const unsigned short &module, const unsigned short &channel) override;
    void MeasureBaseline(const unsigned short &module) override;
    int CopyDSPParameters(const unsigned short &BitMap, const unsigned short &sourceModule,
                          const unsigned short &sourceChannel, unsigned short *DestinationMask) override;
    bool WriteSettings(const char *fname) override;

private:
    explicit XIAInterfaceRemote(size_t num_modules, std::unique_ptr<QTcpSocket> socket);

    QJsonObject rpc(const QJsonObject &req);
    void ensure_ok(const QJsonObject &resp);

    std::unique_ptr<QTcpSocket> sock_;
    QByteArray rxbuf_;
    int req_id_ = 0;
};

#endif
