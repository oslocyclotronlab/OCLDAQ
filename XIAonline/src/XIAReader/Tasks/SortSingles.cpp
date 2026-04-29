//
// Created by Vetle Wegner Ingeberg on 29/04/2026.
//

#include "SortSingles.h"

#include "Trigger.h"

using namespace Task::Singles;

Detector_Histograms_t::Detector_Histograms_t(SharedHistograms &hm, const std::string &name, const size_t &num)
    : energy( hm.Create2D(std::string("energy_"+name), std::string("Energy spectra "+name), 65536, 0, 65536, "Energy [ch]", num, 0, num, std::string(name+" ID")) )
    , energy_CFDfail( hm.Create2D(std::string("energy_"+name), std::string("Energy spectra "+name +" CFD fail"), 65536, 0, 65536, "Energy [ch]", num, 0, num, std::string(name+" ID")) )
    , energy_cal( hm.Create2D(std::string("energy_cal_"+name), std::string("energy spectra "+name+" (cal)"), 16384, 0, 16384, "Energy [keV]", num, 0, num, std::string(name+" ID")) )
    , mult( hm.Create1D(std::string("mult_"+name), std::string("Multiplicity " + name), 128, 0, 128, "Multiplicity") )
{}

void Detector_Histograms_t::Fill(const Entry_t &word)
{
    energy->Fill(word.adcvalue, word.detectorID);
    energy_cal->Fill(word.energy, word.detectorID);
    if ( word.cfdfail )
        energy_CFDfail->Fill(word.cfdfail, word.detectorID);
}

void Detector_Histograms_t::Fill(const subvector<Entry_t> &subvec)
{
    mult->Fill(subvec.size());
    for ( auto &entry : subvec ) {
        Fill(entry);
    }
}

HistManager::HistManager(SharedHistograms &histograms, const UserConfiguration &user_config)
        : configuration( user_config )
        , labr( histograms, "labr_singles", user_config.GetNumDetectors(DetectorType::labr) )
        , si_de( histograms, "si_de_singles", user_config.GetNumDetectors(DetectorType::deDet) )
        , si_e( histograms, "si_e_singles", user_config.GetNumDetectors(DetectorType::eDet) )
        , oscar_F( histograms, "oscar_front_singles", user_config.GetNumDetectors(DetectorType::oscarF) )
        , oscar_B( histograms, "oscar_back_singles", user_config.GetNumDetectors(DetectorType::oscarB) )
        , ppacs( histograms, "ppac_singles", user_config.GetNumDetectors(DetectorType::ppac) )
        , chargeIntegrator( histograms.Create1D("chargeIntegrator", "Charge integrator", 86400, 0, 86400, "Time [ns]") )
{
}

Detector_Histograms_t *HistManager::GetSpec(const DetectorType &type)
{
    switch ( type ) {
        case DetectorType::labr : return &labr;
        case DetectorType::deDet : return &si_de;
        case DetectorType::eDet : return &si_e;
        case DetectorType::oscarF : return &oscar_F;
        case DetectorType::oscarB : return &oscar_B;
        case DetectorType::ppac : return &ppacs;
        default : return nullptr;
    }
}

void HistManager::AddEntry(Triggered_event &buffer)
{

    // We get the qint and increment the time spectrum.
    for ( const auto& Qint : buffer.GetDetector(DetectorType::qint) ) {
        auto time = double(Qint.timestamp) / 1e9; // Convert to second
        chargeIntegrator->Fill(time);
    }


    for ( auto &type : {DetectorType::labr, DetectorType::deDet, DetectorType::eDet, DetectorType::oscarF, DetectorType::oscarB, DetectorType::ppac} ){
        auto spec = GetSpec(type);
        if ( spec )
            spec->Fill(buffer.GetDetector(type));
    }
}

Sorter::Sorter(SharedHistograms& histograms, MCEventQueue_t &input, const UserConfiguration& config)
    : analyzer(histograms, config)
    , input_queue( input )
    , output_queue( )
    , configuration( config )
{
}

void Sorter::Run() {
    std::vector<Entry_t> entries;
    while ( input_queue.is_not_finish() || !input_queue.empty() ) {
        if ( !input_queue.try_pop(entries) ) {
            std::this_thread::yield();
            continue;
        }
        Triggered_event event(entries);
        analyzer.AddEntry(event);
        ++entries_processed;
    }
}