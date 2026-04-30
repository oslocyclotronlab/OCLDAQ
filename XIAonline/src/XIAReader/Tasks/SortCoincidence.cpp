//
// Created by Vetle Wegner Ingeberg on 06/03/2026.
//

#include "SortCoincidence.h"


using namespace Task::Coincidence;

Detector_Histograms_t::Detector_Histograms_t(SharedHistograms &hm, const std::string &name, const size_t &num)
    : time( hm.Create2D(std::string("time_"+name), std::string("Time spectra "+name), 30000, -1500, 1500, "Time [ns]", num, 0, num, std::string(name+" ID")) )
    , time_CFDfail( hm.Create2D(std::string("time_"+name+"_CFDfail"), std::string("Time spectra"+name+" CFD fail"), 30000, -1500, 1500, "Time [ns]", num, 0, num, std::string(name+" ID")) )
    , energy( hm.Create2D(std::string("energy_"+name), std::string("Energy spectra "+name), 65536, 0, 65536, "Energy [ch]", num, 0, num, std::string(name+" ID")) )
    , energy_cal( hm.Create2D(std::string("energy_cal_"+name), std::string("energy spectra "+name+" (cal)"), 16384, 0, 16384, "Energy [keV]", num, 0, num, std::string(name+" ID")) )
    , mult( hm.Create1D(std::string("mult_"+name), std::string("Multiplicity " + name), 128, 0, 128, "Multiplicity") )
{}

void Detector_Histograms_t::Fill(const Entry_t &word)
{
    energy->Fill(word.adcvalue, word.detectorID);
    energy_cal->Fill(word.energy, word.detectorID);
}

void Detector_Histograms_t::Fill(const subvector<Entry_t> &subvec,
                                 const Entry_t *start)
{
    mult->Fill(subvec.size());
    for ( auto &entry : subvec ){
        energy->Fill(entry.adcvalue, entry.detectorID);
        energy_cal->Fill(entry.energy, entry.detectorID);

        if ( start && !( (entry.type == start->type)&&(entry.detectorID == start->detectorID)&&(entry.timestamp==start->timestamp)) ){
            if ( entry.cfdfail ) {
                time_CFDfail->Fill(double(entry.timestamp - start->timestamp) + (entry.cfdcorr - start->cfdcorr), entry.detectorID + 0.5);
            } else {
                time->Fill(double(entry.timestamp - start->timestamp) + (entry.cfdcorr - start->cfdcorr), entry.detectorID + 0.5);
            }
        }
    }
}

Particle_telescope_t::Particle_telescope_t(SharedHistograms &hm, const size_t &num)
        : ede_spectra{hm.Create2D("ede_spectra_b"+std::to_string(num)+"f0", "E energy vs dE energy",
                                  3000, 0, 30000, "E energy [keV]", 1500, 0, 15000, "dE energy [keV]"),
                      hm.Create2D("ede_spectra_b"+std::to_string(num)+"f1", "E energy vs dE energy",
                                  3000, 0, 30000, "E energy [keV]", 1500, 0, 15000, "dE energy [keV]"),
                      hm.Create2D("ede_spectra_b"+std::to_string(num)+"f2", "E energy vs dE energy",
                                  3000, 0, 30000, "E energy [keV]", 1500, 0, 15000, "dE energy [keV]"),
                      hm.Create2D("ede_spectra_b"+std::to_string(num)+"f3", "E energy vs dE energy",
                                  3000, 0, 30000, "E energy [keV]", 1500, 0, 15000, "dE energy [keV]"),
                      hm.Create2D("ede_spectra_b"+std::to_string(num)+"f4", "E energy vs dE energy",
                                  3000, 0, 30000, "E energy [keV]", 1500, 0, 15000, "dE energy [keV]"),
                      hm.Create2D("ede_spectra_b"+std::to_string(num)+"f5", "E energy vs dE energy",
                                  3000, 0, 30000, "E energy [keV]", 1500, 0, 15000, "dE energy [keV]"),
                      hm.Create2D("ede_spectra_b"+std::to_string(num)+"f6", "E energy vs dE energy",
                                  3000, 0, 30000, "E energy [keV]", 1500, 0, 15000, "dE energy [keV]"),
                      hm.Create2D("ede_spectra_b"+std::to_string(num)+"f7", "E energy vs dE energy",
                                  3000, 0, 30000, "E energy [keV]", 2048, 0, 16384, "dE energy [keV]")}
        , ede_spectra_raw{ hm.Create2D("ede_spectra_raw_b"+std::to_string(num)+"f0", "E energy vs dE energy (raw, uncalibrated)",
                                   2048, 0, 16384, "E energy [ch]", 2048, 0, 16384, "dE energy [ch]"),
                           hm.Create2D("ede_spectra_raw_b"+std::to_string(num)+"f1", "E energy vs dE energy (raw, uncalibrated)",
                                   2048, 0, 16384, "E energy [ch]", 2048, 0, 16384, "dE energy [ch]"),
                           hm.Create2D("ede_spectra_raw_b"+std::to_string(num)+"f2", "E energy vs dE energy (raw, uncalibrated)",
                                   2048, 0, 16384, "E energy [ch]", 2048, 0, 16384, "dE energy [ch]"),
                           hm.Create2D("ede_spectra_raw_b"+std::to_string(num)+"f3", "E energy vs dE energy (raw, uncalibrated)",
                                   2048, 0, 16384, "E energy [ch]", 2048, 0, 16384, "dE energy [ch]"),
                           hm.Create2D("ede_spectra_raw_b"+std::to_string(num)+"f4", "E energy vs dE energy (raw, uncalibrated)",
                                   2048, 0, 16384, "E energy [ch]", 2048, 0, 16384, "dE energy [ch]"),
                           hm.Create2D("ede_spectra_raw_b"+std::to_string(num)+"f5", "E energy vs dE energy (raw, uncalibrated)",
                                   2048, 0, 16384, "E energy [ch]", 2048, 0, 16384, "dE energy [ch]"),
                           hm.Create2D("ede_spectra_raw_b"+std::to_string(num)+"f6", "E energy vs dE energy (raw, uncalibrated)",
                                   2048, 0, 16384, "E energy [ch]", 2048, 0, 16384, "dE energy [ch]"),
                           hm.Create2D("ede_spectra_raw_b"+std::to_string(num)+"f7", "E energy vs dE energy (raw, uncalibrated)",
                                   2048, 0, 16384, "E energy [ch]", 2048, 0, 16384, "dE energy [ch]")}
{
}

void Particle_telescope_t::Fill(const subvector<Entry_t> &deltaE, const subvector<Entry_t> &E) {
    for ( const auto &de : deltaE ){
        for ( const auto &e : E ){

            // Check if the same trapzoid. If not, then we will just skip.
            if ( de.detectorID / NUM_SI_DE_TEL != e.detectorID )
                continue;

            ede_spectra[de.detectorID % NUM_SI_DE_TEL]->Fill(e.energy, de.energy);
            ede_spectra_raw[de.detectorID % NUM_SI_DE_TEL]->Fill(e.adcvalue, de.adcvalue);
        }
    }
}

void Particle_telescope_t::Fill(const std::vector<Entry_t> &deltaE, const std::vector<Entry_t> &E)
{
    for ( const auto &de : deltaE ){
        for ( const auto &e : E ){

            // Check if the same trapzoid. If not, then we will just skip.
            if ( de.detectorID / NUM_SI_DE_TEL != e.detectorID )
                continue;

            ede_spectra[de.detectorID % NUM_SI_DE_TEL]->Fill(e.energy, de.energy);
            ede_spectra_raw[de.detectorID % NUM_SI_DE_TEL]->Fill(e.adcvalue, de.adcvalue);
        }
    }
}

HistManager::HistManager(SharedHistograms &histograms, const UserConfiguration &user_config)
    : configuration( user_config )
    , labr( histograms, "labr", user_config.GetNumDetectors(DetectorType::labr) )
    , si_de( histograms, "si_de", user_config.GetNumDetectors(DetectorType::deDet) )
    , si_e( histograms, "si_e", user_config.GetNumDetectors(DetectorType::eDet) )
    , oscar_F( histograms, "oscar_front", user_config.GetNumDetectors(DetectorType::oscarF) )
    , oscar_B( histograms, "oscar_back", user_config.GetNumDetectors(DetectorType::oscarB) )
    , ppacs( histograms, "ppac", user_config.GetNumDetectors(DetectorType::ppac) )
    , particle_coincidence{{ histograms, 0}, { histograms, 1}, { histograms, 2}, { histograms, 3},
                           { histograms, 4}, { histograms, 5}, { histograms, 6},{ histograms, 7}}
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

void HistManager::AddEntry(Triggered_event &buffer) {
    const auto *trigger = buffer.GetTrigger();

    // No trigger, we cannot analyze the data.
    if ( !trigger ) {
        return;
    }

    // For now, we will discard events with bad CFD
    // We have this req. if we get a trigger
    if ( trigger->cfdfail )
        return;

    for ( auto &type : {DetectorType::labr, DetectorType::deDet, DetectorType::eDet, DetectorType::oscarF, DetectorType::oscarB, DetectorType::ppac} ){
        auto spec = GetSpec(type);
        if ( spec )
            spec->Fill(buffer.GetDetector(type), trigger);
    }

    if ( trigger->type != DetectorType::deDet ) // All analysis below requires the trigger to be a dE detector
        return;

    auto trapID = trigger->detectorID / 8;
    auto ringID = trigger->detectorID % 8;
    auto [de_evts, e_evts] = buffer.GetTrap(trapID);

}

Sorter::Sorter(SharedHistograms& histograms, TEventQueue_t &input, const UserConfiguration& config)
    : analyzer(histograms, config)
    , input_queue( input )
    , configuration( config )
{
}

void Sorter::Run() {
    std::pair<std::vector<Entry_t>, int> entries;
    while ( input_queue.is_not_finish() || !input_queue.empty() ) {
        if ( !input_queue.try_pop(entries) ) {
            std::this_thread::yield();
            continue;
        }
        ++entries_processed;
    }
}