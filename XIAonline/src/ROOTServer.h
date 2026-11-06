//
// Created by Vetle Wegner Ingeberg on 29/04/2026.
//

#ifndef ROOTSERVER_H
#define ROOTSERVER_H

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <histogram/SharedHistograms.h>
#include <THttpServer.h>

#include "elog_client.h"

struct Options_t;

class TH1;
class TH2;
class TH3;
class THttpCallArg;

//! Elog settings for the online server, taken from acq_master_commands.txt.
struct OnlineElogSettings {
    elog_config config;                   //!< where to submit entries
    std::string exp_id;                   //!< current experiment id, for the entry text
    std::size_t max_attachment = 50 << 20; //!< refuse attachments larger than this (bytes)
};

//! THttpServer that serves our own main page (with elog buttons) and
//! handles the elog/... requests itself. Everything else is passed on.
class OnlineHttpServer : public THttpServer {
public:
    typedef std::function<bool(std::shared_ptr<THttpCallArg>&)> handler_t;

    OnlineHttpServer(const char* engine, const char* page, handler_t handler);

protected:
    void ProcessRequest(std::shared_ptr<THttpCallArg> arg) override;

private:
    handler_t handler;
};

class ROOTServer {

public:
    typedef std::vector<std::pair<TH1*,SharedHistogram1Dp>> list1D_t;
    typedef std::vector<std::pair<TH2*,SharedHistogram2Dp>> list2D_t;
    typedef std::vector<std::pair<TH3*,SharedHistogram3Dp>> list3D_t;

    ROOTServer(const Options_t& options, SharedHistograms& hists, const char& leaveprog);
    ~ROOTServer() = default;
    void run();

    //! Update the elog settings (called from the main thread, e.g. after change_cwd).
    void SetElogSettings(const OnlineElogSettings& settings);

private:
    const char* leaveprog_ptr;
    SharedHistograms& histogram_manager;
    OnlineHttpServer server;

    list1D_t histograms;
    list2D_t matrices;
    list3D_t cubes;

    //! All registered ROOT histograms, keyed by "<path>/<name>" (no leading '/').
    std::map<std::string, TH1*> items;
    //! Folder (registration path) of each histogram, for writing ROOT files.
    std::map<TH1*, std::string> folders;

    const bool elog_allowed;           //!< false if disabled on the command line
    std::mutex elog_mutex;             //!< protects elog_settings
    OnlineElogSettings elog_settings;

    //! Submissions in progress / finished. Shared with the submit threads,
    //! which may outlive this object.
    struct ElogJobs;
    std::shared_ptr<ElogJobs> elog_jobs;

    void update();

    //! Called for each HTTP request in the server thread. Returns true if handled.
    bool HandleRequest(std::shared_ptr<THttpCallArg>& arg);

    void ElogConfig(std::shared_ptr<THttpCallArg>& arg);
    void ElogSubmit(std::shared_ptr<THttpCallArg>& arg);
    void ElogSubmitAll(std::shared_ptr<THttpCallArg>& arg);
    void ElogStatus(std::shared_ptr<THttpCallArg>& arg);

    //! Start an asynchronous submission, return the job id.
    int StartElogJob(const OnlineElogSettings& settings, const elog_attributes& attrs,
                     const std::string& text, const elog_attachments& files);

    TH1* FindItem(const std::string& item, const std::string& name) const;
};

#endif // ROOTSERVER_H
