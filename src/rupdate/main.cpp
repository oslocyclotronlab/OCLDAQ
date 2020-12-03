//
// Created by Vetle Wegner Ingeberg on 02/12/2020.
//

#include "rupdate.h"
#include "io_root.h"
#include "sort_spectra.h"

#include <iostream>
#include <unistd.h>

#include <TApplication.h>
#include <TStyle.h>


int main(int argc, char* argv[])
{
    TApplication theApp("ROOTupdate", &argc, argv);

    gStyle->SetPadGridX(kTRUE);
    gStyle->SetPadGridY(kTRUE);
    gStyle->SetPalette(1);
    gStyle->SetCanvasColor(10);
    gStyle->SetOptStat(1111111);
    gStyle->SetStatFormat("10.8g");
    gStyle->SetNdivisions(520,"y");

    // ------------------------------

    int maxupdate = 2;

    for( int opt=0; (opt = getopt(argc, argv, "u:")) != -1; ) {
        switch (opt) {
            case 'u':
                maxupdate = atoi(optarg);
                break;
            default: /* '?' */
                std::cerr << "Usage: " << argv[0] << "[-u update_secs]" << std::endl;
                exit(EXIT_FAILURE);
        }
    }

    // ------------------------------

    spectra_attach_all(true);
    new MyMainFrame(maxupdate);
    theApp.Run(true);
    spectra_detach_all();

    return 0;
}