//
// Created by Vetle Wegner Ingeberg on 29/04/2026.
//

#include "ROOTServer.h"

#include <histogram/SharedHistograms.h>

#include <TH1.h>
#include <TH2.h>
#include <TH3.h>

#include "options.h"


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

ROOTServer::ROOTServer(const Options_t &options, const char& _leaveprog)
    : leaveprog( _leaveprog )
    , histogram_manager( SharedHistograms::Attach(options.shared_histograms_name.value()) )
    , server( setup_server_options(options.engine.value().c_str(), options.bind_address.value().c_str(), options.bind_port.value()).c_str() )
    , histograms( MapHist(histogram_manager.GetAll1D()) )
    , matrices( MapMat(histogram_manager.GetAll2D()) )
    , cubes( MapCube(histogram_manager.GetAll3D()) )
{
    // Register all histograms
    for ( auto& hist : histograms) {
        server.Register("/", hist.first);
    }
    // Register all matrices
    for ( auto& mat : matrices) {
        server.Register("/", mat.first);
    }
    // Register all cubes
    for ( auto& cube : cubes) {
        server.Register("/", cube.first);
    }
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
    while (leaveprog == 'n') {
        server.ProcessRequests();
        update();
    }
}
