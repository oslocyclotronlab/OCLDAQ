//
// Created by Vetle Wegner Ingeberg on 06/03/2026.
//

#include "SortCoincidence.h"


using namespace Task::Coincidence;

HistManager::HistManager(SharedHistograms &histograms, const UserConfiguration &user_config)
    : configuration( user_config )
    , labr( histograms, "labr", user_config.GetNumDetectors(DetectorType::labr) )
    , si_de( histograms, "si_de", user_config.GetNumDetectors(DetectorType::deDet) )
    , si_e( histograms, "si_e", user_config.GetNumDetectors(DetectorType::eDet) )
    , oscar_F( histograms, "oscar_front_singles", user_config.GetNumDetectors(DetectorType::oscarF) )
    , oscar_B( histograms, "oscar_back_singles", user_config.GetNumDetectors(DetectorType::oscarB) )
    , ppacs( histograms, "ppac", user_config.GetNumDetectors(DetectorType::ppac) )
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