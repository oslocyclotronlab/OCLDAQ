//
// Created by Vetle Wegner Ingeberg on 29/04/2026.
//

#ifndef ROOTSERVER_H
#define ROOTSERVER_H

#include <memory>
#include <vector>

#include <histogram/SharedHistograms.h>
#include <THttpServer.h>

struct Options_t;

class TH1;
class TH2;
class TH3;

class ROOTServer {

public:
    typedef std::vector<std::pair<TH1*,SharedHistogram1Dp>> list1D_t;
    typedef std::vector<std::pair<TH2*,SharedHistogram2Dp>> list2D_t;
    typedef std::vector<std::pair<TH3*,SharedHistogram3Dp>> list3D_t;

    ROOTServer(const Options_t& options, const char& leaveprog);
    ~ROOTServer() = default;
    void run();

private:
    const char leaveprog;
    SharedHistograms histogram_manager;
    THttpServer server;

    list1D_t histograms;
    list2D_t matrices;
    list3D_t cubes;

    void update();



};

#endif // ROOTSERVER_H
