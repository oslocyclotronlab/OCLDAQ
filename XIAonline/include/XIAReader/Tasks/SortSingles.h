//
// Created by Vetle Wegner Ingeberg on 29/04/2026.
//

#ifndef SORTSINGLES_H
#define SORTSINGLES_H

#include <Tasks/Task.h>
#include <Tasks/Queue.h>
#include <Configuration/UserConfiguration.h>

#include <histogram/SharedHistograms.h>

namespace Task {
    namespace Singles {
        struct Detector_Histograms_t
        {
            SharedHistogram2Dp energy;
            SharedHistogram2Dp energy_CFDfail;
            SharedHistogram2Dp energy_cal;
            SharedHistogram1Dp mult;

            Detector_Histograms_t(SharedHistograms &hist, const std::string &name, const size_t &num);

            void Fill(const Entry_t &word);
            void Fill(const subvector<Entry_t> &subvec);

        };

        class HistManager {
        private:
            const UserConfiguration configuration;

            Detector_Histograms_t labr;
            Detector_Histograms_t si_de;
            Detector_Histograms_t si_e;
            Detector_Histograms_t oscar_F;
            Detector_Histograms_t oscar_B;
            Detector_Histograms_t ppacs;

            SharedHistogram1Dp chargeIntegrator;

            Detector_Histograms_t *GetSpec(const DetectorType &type);

        public:
            HistManager(SharedHistograms &histograms, const UserConfiguration &configuration);
            ~HistManager() = default;

            //! Fill spectra with an event
            void AddEntry(Triggered_event &buffer);

            //! Fill a single word
            //void AddEntry(const Entry_t &word);

            //! Fill spectra directly from iterators
            template<class It>
            inline void AddEntries(It start, It stop){
                using std::placeholders::_1;
                std::for_each(start, stop, [this](const auto &p){ this->AddEntry(p); });
            }

        };

        class Sorter  : public Base
        {
        private:
            HistManager analyzer;
            MCEventQueue_t &input_queue;
            MCEventQueue_t output_queue;
            const UserConfiguration& configuration;


        public:
            CLASS_NAME(Sorter)
            Sorter(SharedHistograms& histograms, MCEventQueue_t &input, const UserConfiguration& config);
            ~Sorter() override = default;
            MCEventQueue_t &GetQueue(){ return output_queue; }
            void Run() override;
        };
    }
};

#endif //SORTSINGLES_H
