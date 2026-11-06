//
// Created by Vetle Wegner Ingeberg on 29/04/2026.
//

#include "ROOTServer.h"

#include <histogram/SharedHistograms.h>

#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <THttpCallArg.h>
#include <TMemFile.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <ctime>
#include <sstream>

#include <unistd.h>

#include "options.h"
#include "OnlinePage.h"


TH1* Setup1D(SharedHistogram1Dp& shared_hist) {
    if ( shared_hist.get() == nullptr)
        return nullptr;


    const Axis& xax = shared_hist->GetAxisX();
    const int channels = xax.GetBinCount();
    TH1* r = new TH1I( shared_hist->GetName().c_str(), shared_hist->GetTitle().c_str(),
                                        channels, xax.GetLeft(), xax.GetRight() );

    TAxis* rxax = r->GetXaxis();
    rxax->SetTitle(xax.GetTitle().c_str());
    rxax->SetTitleSize(0.03);
    rxax->SetLabelSize(0.03);

    TAxis* ryax = r->GetYaxis();
    ryax->SetLabelSize(0.03);

    for(int i=0; i<channels+2; ++i)
        r->SetBinContent(i, shared_hist->GetBinContent(i));
    r->SetEntries( shared_hist->GetEntries() );
    return r;
}

TH2* Setup2D(SharedHistogram2Dp& shared_hist) {
        const Axis& xax = shared_hist->GetAxisX();
        const Axis& yax = shared_hist->GetAxisY();
        const int xchannels = xax.GetBinCount();
        const int ychannels = yax.GetBinCount();
        TH2* mat = new TH2F( shared_hist->GetName().c_str(), shared_hist->GetTitle().c_str(),
                             xchannels, xax.GetLeft(), xax.GetRight(),
                             ychannels, yax.GetLeft(), yax.GetRight() );
        mat->SetOption( "colz" );
        mat->SetContour( 64 );

        TAxis* rxax = mat->GetXaxis();
        rxax->SetTitle(xax.GetTitle().c_str());
        rxax->SetTitleSize(0.03);
        rxax->SetLabelSize(0.03);

        TAxis* ryax = mat->GetYaxis();
        ryax->SetTitle(yax.GetTitle().c_str());
        ryax->SetTitleSize(0.03);
        ryax->SetLabelSize(0.03);
        ryax->SetTitleOffset(1.3);

        TAxis* zax = mat->GetZaxis();
        zax->SetLabelSize(0.025);

        for(int iy=0; iy<ychannels+2; ++iy)
            for(int ix=0; ix<xchannels+2; ++ix)
                mat->SetBinContent(ix, iy, shared_hist->GetBinContent(ix, iy));
        mat->SetEntries( shared_hist->GetEntries() );

        return mat;
    }


TH3* Setup3D(SharedHistogram3Dp& shared_hist) {
    const Axis& xax = shared_hist->GetAxisX();
    const Axis& yax = shared_hist->GetAxisY();
    const Axis& zax = shared_hist->GetAxisZ();
    const auto xchannels = xax.GetBinCount();
    const auto ychannels = yax.GetBinCount();
    const auto zchannels = zax.GetBinCount();
    TH3* cube = new TH3F( shared_hist->GetName().c_str(), shared_hist->GetTitle().c_str(),
                          xchannels, xax.GetLeft(), xax.GetRight(),
                          ychannels, yax.GetLeft(), yax.GetRight(),
                         zchannels, zax.GetLeft(), zax.GetRight());
    cube->SetOption( "colz" );
    cube->SetContour( 64 );

    TAxis* rxax = cube->GetXaxis();
    rxax->SetTitle(xax.GetTitle().c_str());
    rxax->SetTitleSize(0.03);
    rxax->SetLabelSize(0.03);

    TAxis* ryax = cube->GetYaxis();
    ryax->SetTitle(yax.GetTitle().c_str());
    ryax->SetTitleSize(0.03);
    ryax->SetLabelSize(0.03);
    ryax->SetTitleOffset(1.3);

    TAxis* rzax = cube->GetZaxis();
    rzax->SetTitle(zax.GetTitle().c_str());
    rzax->SetLabelSize(0.025);

    for(Axis::index_t iz=0; iz<zchannels+2; ++iz)
        for(Axis::index_t iy=0; iy<ychannels+2; ++iy)
            for(Axis::index_t ix=0; ix<xchannels+2; ++ix)
                cube->SetBinContent(ix, iy, iz, shared_hist->GetBinContent(ix, iy, iz));
    cube->SetEntries( shared_hist->GetEntries() );

    return cube;
}

ROOTServer::list1D_t MapHist(SharedHistograms::list1d_t hists) {
    ROOTServer::list1D_t v;
    for ( auto& hist : hists) {
        v.emplace_back(Setup1D(hist), hist);
    }
    return v;
}

ROOTServer::list2D_t MapMat(SharedHistograms::list2d_t hists) {
    ROOTServer::list2D_t v;
    for ( auto& hist : hists) {
        v.emplace_back(Setup2D(hist), hist);
    }
    return v;
}

ROOTServer::list3D_t MapCube(SharedHistograms::list3d_t hists) {
    ROOTServer::list3D_t v;
    for ( auto& hist : hists) {
        v.emplace_back(Setup3D(hist), hist);
    }
    return v;
}

void FillTH1(TH1* rhist, SharedHistogram1Dp &lhist) {
    if (rhist == nullptr) {
        LFLOG_DEBUG << "FillTH1: rhist == nullptr";
    }
    if (lhist.get() == nullptr) {
        LFLOG_DEBUG << "FillTH1: lhist == nullptr";
    }

    for(Axis::index_t ix=0; ix< lhist->GetAxisX().GetBinCount()+2; ++ix)
        rhist->SetBinContent(ix, lhist->GetBinContent(ix));
    rhist->SetEntries( lhist->GetEntries() );
}

void FillTH2(TH2* rhist, SharedHistogram2Dp &lhist) {
    if (rhist == nullptr) {
        LFLOG_DEBUG << "FillTH1: rhist == nullptr";
    }
    if (lhist.get() == nullptr) {
        LFLOG_DEBUG << "FillTH1: lhist == nullptr";
    }

    for(Axis::index_t iy=0; iy< lhist->GetAxisY().GetBinCount()+2; ++iy)
        for(Axis::index_t ix=0; ix< lhist->GetAxisX().GetBinCount()+2; ++ix)
        rhist->SetBinContent(ix, iy, lhist->GetBinContent(ix, iy));
    rhist->SetEntries( lhist->GetEntries() );
}

void FillTH3(TH3* rhist, SharedHistogram3Dp &lhist) {
    if (rhist == nullptr) {
        LFLOG_DEBUG << "FillTH1: rhist == nullptr";
    }
    if (lhist.get() == nullptr) {
        LFLOG_DEBUG << "FillTH1: lhist == nullptr";
    }
    for(Axis::index_t iz=0; iz< lhist->GetAxisZ().GetBinCount()+2; ++iz)
        for(Axis::index_t iy=0; iy< lhist->GetAxisY().GetBinCount()+2; ++iy)
            for(Axis::index_t ix=0; ix< lhist->GetAxisX().GetBinCount()+2; ++ix)
                rhist->SetBinContent(ix, iy, iz, lhist->GetBinContent(ix, iy, iz));
    rhist->SetEntries( lhist->GetEntries() );
}

std::string setup_server_options(const char* engine, const char* bind_address, const int& port_number) {
    std::string options;
    options = engine;
    options += ":" + std::string(bind_address);
    options += ":" + std::to_string(port_number);
    return options;
}

// ########################################################################
// small helpers for the elog requests
// ########################################################################

namespace {

const std::size_t MAX_PNG_SIZE = 20u << 20;   // largest screenshot we accept
const std::size_t MAX_FINISHED_JOBS = 100;    // forget old results beyond this

std::string url_decode(const std::string& in)
{
    std::string out;
    out.reserve(in.size());
    for(std::size_t i=0; i<in.size(); ++i) {
        if( in[i] == '+' ) {
            out += ' ';
        } else if( in[i] == '%' && i+2 < in.size()
                   && std::isxdigit(static_cast<unsigned char>(in[i+1]))
                   && std::isxdigit(static_cast<unsigned char>(in[i+2])) ) {
            out += static_cast<char>(std::stoi(in.substr(i+1, 2), nullptr, 16));
            i += 2;
        } else {
            out += in[i];
        }
    }
    return out;
}

std::map<std::string, std::string> parse_query(const char* query)
{
    std::map<std::string, std::string> result;
    std::istringstream in(query ? query : "");
    std::string pair;
    while( std::getline(in, pair, '&') ) {
        if( pair.empty() )
            continue;
        const auto eq = pair.find('=');
        if( eq == std::string::npos )
            result[url_decode(pair)] = "";
        else
            result[url_decode(pair.substr(0, eq))] = url_decode(pair.substr(eq+1));
    }
    return result;
}

std::string json_escape(const std::string& in)
{
    std::string out;
    for(char c : in) {
        switch( c ) {
        case '"':  out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\n': out += "\\n";  break;
        case '\r': out += "\\r";  break;
        case '\t': out += "\\t";  break;
        default:
            if( static_cast<unsigned char>(c) < 0x20 ) {
                char buf[8];
                std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                out += buf;
            } else {
                out += c;
            }
        }
    }
    return out;
}

//! Remove leading/trailing and repeated '/'.
std::string normalize_path(const std::string& path)
{
    std::string out;
    for(char c : path) {
        if( c == '/' && (out.empty() || out.back() == '/') )
            continue;
        out += c;
    }
    while( !out.empty() && out.back() == '/' )
        out.pop_back();
    return out;
}

std::string item_key(const std::string& path, const std::string& name)
{
    const std::string p = normalize_path(path);
    return p.empty() ? name : p + "/" + name;
}

//! Keep only characters that are safe in a file name.
std::string safe_filename(const std::string& in)
{
    std::string out;
    for(char c : in)
        out += (std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' || c == '.') ? c : '_';
    return out.empty() ? std::string("spectrum") : out;
}

std::string timestamp(const char* format)
{
    char buf[64];
    const std::time_t now = std::time(nullptr);
    std::tm tm_now;
    localtime_r(&now, &tm_now);
    std::strftime(buf, sizeof(buf), format, &tm_now);
    return buf;
}

std::string current_dir()
{
    char cwd[4096];
    return getcwd(cwd, sizeof(cwd)) ? std::string(cwd) : std::string("-unknown-");
}

void reply_json(std::shared_ptr<THttpCallArg>& arg, const std::string& json)
{
    arg->SetContentType("application/json");
    arg->AddNoCacheHeader();
    arg->SetContent(std::string(json));
}

void reply_error(std::shared_ptr<THttpCallArg>& arg, const std::string& error)
{
    LFLOG_WARN << "elog: " << error;
    reply_json(arg, "{\"ok\":false,\"error\":\"" + json_escape(error) + "\"}");
}

//! Write the given histograms into an in-memory ROOT file and return its bytes.
std::string make_root_file(const std::vector<std::pair<TH1*, std::string>>& hists)
{
    TDirectory::TContext ctx; // restore gDirectory when done
    TMemFile mf("online.root", "RECREATE", "XIAonline spectra");
    for(const auto& h : hists) {
        TDirectory* dir = &mf;
        const std::string folder = normalize_path(h.second);
        if( !folder.empty() )
            dir = mf.mkdir(folder.c_str(), "", true);
        if( dir )
            dir->WriteTObject(h.first, h.first->GetName());
    }
    mf.Write();
    mf.Close();

    std::string bytes(static_cast<std::size_t>(mf.GetEND()), '\0');
    mf.CopyTo(bytes.data(), static_cast<Long64_t>(bytes.size()));
    return bytes;
}

std::string describe_view(const std::map<std::string, std::string>& q)
{
    std::ostringstream out;
    auto get = [&q](const char* key) {
        auto it = q.find(key);
        return it == q.end() ? std::string() : it->second;
    };
    if( !get("xmin").empty() )
        out << "x range: [" << get("xmin") << ", " << get("xmax") << "]\n";
    if( !get("ymin").empty() )
        out << "y range: [" << get("ymin") << ", " << get("ymax") << "]\n";
    std::string logs;
    for(const char* axis : {"logx", "logy", "logz"})
        if( get(axis) == "1" )
            logs += std::string(logs.empty() ? "" : ", ") + axis;
    if( !logs.empty() )
        out << "log scale: " << logs << "\n";
    if( !get("option").empty() )
        out << "draw option: " << get("option") << "\n";
    return out.str();
}

} // namespace

// ########################################################################

struct ROOTServer::ElogJobs {
    struct Job {
        bool done = false;
        elog_result result;
    };
    std::mutex mutex;
    int next_id = 1;
    std::map<int, Job> jobs;
};

// ########################################################################

OnlineHttpServer::OnlineHttpServer(const char* engine, const char* page, handler_t h)
    : THttpServer(engine)
    , handler(std::move(h))
{
    // Serve our own main page. THttpServer only reads fDefaultPage from disk if
    // fDefaultPageCont is empty, so the page can be compiled into the program.
    // The name is only a label.
    if( page && *page ) {
        fDefaultPage = "xiaonline.htm";
        fDefaultPageCont = page;
    }
}

void OnlineHttpServer::ProcessRequest(std::shared_ptr<THttpCallArg> arg)
{
    if( handler && handler(arg) )
        return;
    THttpServer::ProcessRequest(arg);
}

// ########################################################################

ROOTServer::ROOTServer(const Options_t &options, SharedHistograms& hists, const char& _leaveprog)
    : leaveprog_ptr( &_leaveprog )
    , histogram_manager( hists )
    , server( setup_server_options(options.engine.value().c_str(), options.bind_address.value().c_str(), options.bind_port.value()).c_str(),
              XIAONLINE_PAGE,
              [this](std::shared_ptr<THttpCallArg>& arg) { return HandleRequest(arg); } )
    , histograms( MapHist(histogram_manager.GetAll1D()) )
    , matrices( MapMat(histogram_manager.GetAll2D()) )
    , cubes( MapCube(histogram_manager.GetAll3D()) )
    , elog_allowed( !options.disable_elog.value_or(false) )
    , elog_jobs( std::make_shared<ElogJobs>() )
{
    auto add = [this](TH1* h, const std::string& path) {
        if( !h )
            return;
        server.Register(path.c_str(), h);
        items[item_key(path, h->GetName())] = h;
        folders[h] = path;
    };

    // Register all histograms
    for ( auto& hist : histograms)
        add(hist.first, hist.second->GetPath());
    // Register all matrices
    for ( auto& mat : matrices)
        add(mat.first, mat.second->GetPath());
    // Register all cubes
    for ( auto& cube : cubes)
        add(cube.first, cube.second->GetPath());
}

void ROOTServer::SetElogSettings(const OnlineElogSettings& settings)
{
    std::lock_guard<std::mutex> lock(elog_mutex);
    elog_settings = settings;
}

void ROOTServer::update() {
    // Update all histograms
    for ( auto& hist : histograms)
        FillTH1(hist.first, hist.second);
    for ( auto& mat : matrices)
        FillTH2(mat.first, mat.second);
    for ( auto& cube : cubes)
        FillTH3(cube.first, cube.second);

}

void ROOTServer::run() {
    while (*leaveprog_ptr == 'n') {
        server.ProcessRequests();
        update();
    }
}

// ########################################################################
// elog requests. These run in the server thread (inside ProcessRequests),
// the same thread that runs update(), so the ROOT histograms are not
// modified while we read them.
// ########################################################################

bool ROOTServer::HandleRequest(std::shared_ptr<THttpCallArg>& arg)
{
    if( std::string(arg->GetPathName()) != "elog" )
        return false;

    const std::string what = arg->GetFileName();
    if( what == "config" )
        ElogConfig(arg);
    else if( what == "submit" )
        ElogSubmit(arg);
    else if( what == "submit_all" )
        ElogSubmitAll(arg);
    else if( what == "status" )
        ElogStatus(arg);
    else
        return false;
    return true;
}

void ROOTServer::ElogConfig(std::shared_ptr<THttpCallArg>& arg)
{
    OnlineElogSettings settings;
    {
        std::lock_guard<std::mutex> lock(elog_mutex);
        settings = elog_settings;
    }
    const bool enabled = elog_allowed && settings.config.enabled();
    std::ostringstream out;
    out << "{\"enabled\":" << (enabled ? "true" : "false")
        << ",\"logbook\":\"" << json_escape(enabled ? settings.config.describe() : std::string()) << "\""
        << ",\"exp_id\":\"" << json_escape(settings.exp_id) << "\"";
    if( !elog_allowed )
        out << ",\"reason\":\"elog disabled on the XIAonline command line\"";
    else if( !settings.config.enabled() )
        out << ",\"reason\":\"set elog_host and elog_logbook in acq_master_commands.txt\"";
    out << "}";
    reply_json(arg, out.str());
}

TH1* ROOTServer::FindItem(const std::string& item, const std::string& name) const
{
    // JSROOT item names look like "Objects/<path>/<name>": THttpServer puts
    // registered objects into the "Objects" folder.
    std::string key = normalize_path(item);
    auto it = items.find(key);
    if( it == items.end() && key.rfind("Objects/", 0) == 0 )
        it = items.find(key.substr(8));
    if( it != items.end() )
        return it->second;

    // Fall back to the histogram name, if it is unique.
    TH1* found = nullptr;
    for(const auto& i : items) {
        if( name.empty() || name != i.second->GetName() )
            continue;
        if( found )
            return nullptr;
        found = i.second;
    }
    return found;
}

int ROOTServer::StartElogJob(const OnlineElogSettings& settings, const elog_attributes& attrs,
                             const std::string& text, const elog_attachments& files)
{
    std::shared_ptr<ElogJobs> jobs = elog_jobs;
    int id;
    {
        std::lock_guard<std::mutex> lock(jobs->mutex);
        id = jobs->next_id++;
        jobs->jobs[id] = ElogJobs::Job();

        // forget the oldest finished jobs that nobody asked for
        while( jobs->jobs.size() > MAX_FINISHED_JOBS ) {
            auto old = std::find_if(jobs->jobs.begin(), jobs->jobs.end(),
                                    [](const auto& j) { return j.second.done; });
            if( old == jobs->jobs.end() )
                break;
            jobs->jobs.erase(old);
        }
    }

    elog_submit_async(settings.config, attrs, text, files, [jobs, id](const elog_result& res) {
        std::lock_guard<std::mutex> lock(jobs->mutex);
        auto& job = jobs->jobs[id];
        job.done = true;
        job.result = res;
    });
    return id;
}

void ROOTServer::ElogSubmit(std::shared_ptr<THttpCallArg>& arg)
{
    OnlineElogSettings settings;
    {
        std::lock_guard<std::mutex> lock(elog_mutex);
        settings = elog_settings;
    }
    if( !elog_allowed || !settings.config.enabled() )
        return reply_error(arg, "elog is not enabled for this server");
    if( std::string(arg->GetMethod()) != "POST" )
        return reply_error(arg, "elog/submit expects a POST request");

    const auto q = parse_query(arg->GetQuery());
    auto get = [&q](const char* key) {
        auto it = q.find(key);
        return it == q.end() ? std::string() : it->second;
    };

    TH1* h = FindItem(get("item"), get("name"));
    if( !h )
        return reply_error(arg, "unknown spectrum '" + get("item") + "'");

    const std::size_t png_size = static_cast<std::size_t>(arg->GetPostDataLength());
    if( png_size == 0 )
        return reply_error(arg, "no image received");
    if( png_size > MAX_PNG_SIZE )
        return reply_error(arg, "image too large");

    const std::string name = h->GetName();
    const std::string fname = safe_filename(name) + "_" + timestamp("%Y%m%d_%H%M%S");

    elog_attachments files;
    files.emplace_back(fname + ".png",
                       std::string(static_cast<const char*>(arg->GetPostData()), png_size));
    files.emplace_back(fname + ".root", make_root_file({{h, folders[h]}}));
    if( files.back().data.size() > settings.max_attachment )
        return reply_error(arg, "ROOT file of '" + name + "' is too large for the elog");

    std::ostringstream text;
    if( !get("comment").empty() )
        text << get("comment") << "\n\n";
    text << "XIAonline screenshot of " << name;
    if( std::string(h->GetTitle()) != name && *h->GetTitle() )
        text << " (" << h->GetTitle() << ")";
    text << "\n"
         << "Experiment: " << (settings.exp_id.empty() ? "-none-" : settings.exp_id) << "\n"
         << "Directory: " << current_dir() << "\n"
         << "Time: " << timestamp("%Y-%m-%d %H:%M:%S") << "\n"
         << "Entries: " << static_cast<long long>(h->GetEntries()) << "\n"
         << describe_view(q);

    elog_attributes attrs;
    attrs.emplace_back("Subject", "screenshot " + name);

    const int id = StartElogJob(settings, attrs, text.str(), files);
    LFLOG_INFO << "elog: sending screenshot of '" << name << "' to " << settings.config.describe();
    reply_json(arg, "{\"ok\":true,\"job\":" + std::to_string(id) + "}");
}

void ROOTServer::ElogSubmitAll(std::shared_ptr<THttpCallArg>& arg)
{
    OnlineElogSettings settings;
    {
        std::lock_guard<std::mutex> lock(elog_mutex);
        settings = elog_settings;
    }
    if( !elog_allowed || !settings.config.enabled() )
        return reply_error(arg, "elog is not enabled for this server");
    if( std::string(arg->GetMethod()) != "POST" )
        return reply_error(arg, "elog/submit_all expects a POST request");

    const auto q = parse_query(arg->GetQuery());
    const auto comment = q.count("comment") ? q.at("comment") : std::string();

    std::vector<std::pair<TH1*, std::string>> all;
    std::size_t n1 = 0, n2 = 0, n3 = 0;
    for(auto& h : histograms)
        if( h.first ) { all.emplace_back(h.first, h.second->GetPath()); ++n1; }
    for(auto& h : matrices)
        if( h.first ) { all.emplace_back(h.first, h.second->GetPath()); ++n2; }
    for(auto& h : cubes)
        if( h.first ) { all.emplace_back(h.first, h.second->GetPath()); ++n3; }

    const std::string exp = settings.exp_id.empty() ? std::string("-none-") : settings.exp_id;
    const std::string fname = "XIAonline_" + safe_filename(exp) + "_" + timestamp("%Y%m%d_%H%M%S") + ".root";

    elog_attachments files;
    files.emplace_back(fname, make_root_file(all));
    const std::size_t size = files.back().data.size();
    LFLOG_INFO << "elog: ROOT file with all spectra is " << size / 1024 << " kB";
    if( size > settings.max_attachment ) {
        std::ostringstream err;
        err << "ROOT file with all spectra is " << (size >> 20) << " MB, the limit is "
            << (settings.max_attachment >> 20) << " MB (elog_max_attachment_mb)";
        return reply_error(arg, err.str());
    }

    std::ostringstream text;
    if( !comment.empty() )
        text << comment << "\n\n";
    text << "XIAonline: all spectra\n"
         << "Experiment: " << exp << "\n"
         << "Directory: " << current_dir() << "\n"
         << "Time: " << timestamp("%Y-%m-%d %H:%M:%S") << "\n"
         << "Spectra: " << n1 << " 1D, " << n2 << " 2D, " << n3 << " 3D\n";

    elog_attributes attrs;
    attrs.emplace_back("Subject", "all spectra " + exp);

    const int id = StartElogJob(settings, attrs, text.str(), files);
    LFLOG_INFO << "elog: sending all spectra to " << settings.config.describe();
    reply_json(arg, "{\"ok\":true,\"job\":" + std::to_string(id) + "}");
}

void ROOTServer::ElogStatus(std::shared_ptr<THttpCallArg>& arg)
{
    const auto q = parse_query(arg->GetQuery());
    int id = -1;
    try {
        id = q.count("job") ? std::stoi(q.at("job")) : -1;
    } catch( std::exception& ) {
        id = -1;
    }

    std::lock_guard<std::mutex> lock(elog_jobs->mutex);
    auto it = elog_jobs->jobs.find(id);
    if( it == elog_jobs->jobs.end() )
        return reply_error(arg, "unknown elog job");

    if( !it->second.done )
        return reply_json(arg, "{\"ok\":true,\"done\":false}");

    const elog_result res = it->second.result;
    elog_jobs->jobs.erase(it);

    std::ostringstream out;
    out << "{\"ok\":" << (res.ok ? "true" : "false") << ",\"done\":true"
        << ",\"message_id\":" << res.message_id
        << ",\"error\":\"" << json_escape(res.error) << "\""
        << ",\"warnings\":[";
    for(std::size_t i=0; i<res.warnings.size(); ++i)
        out << (i ? "," : "") << "\"" << json_escape(res.warnings[i]) << "\"";
    out << "]}";
    reply_json(arg, out.str());
}
