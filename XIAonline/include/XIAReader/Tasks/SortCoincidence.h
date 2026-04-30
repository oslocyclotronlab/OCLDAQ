//
// Created by Vetle Wegner Ingeberg on 06/03/2026.
//

#ifndef SORT_H
#define SORT_H




#include <Tasks/Task.h>
#include <Tasks/Queue.h>

#include <histogram/SharedHistograms.h>
#include <Configuration/UserConfiguration.h>

namespace Task {
    namespace Coincidence {

        struct Detector_Histograms_t
        {
            SharedHistogram2Dp time;
            SharedHistogram2Dp time_CFDfail;
            SharedHistogram2Dp energy;
            SharedHistogram2Dp energy_CFDfail;
            SharedHistogram2Dp energy_cal;
            SharedHistogram1Dp mult;

            Detector_Histograms_t(SharedHistograms &hist, const std::string &name, const size_t &num);

            void Fill(const Entry_t &word);
            void Fill(const subvector<Entry_t> &subvec,
                      const Entry_t *start = nullptr);

        };

        class Particle_telescope_t
        {
        public:
            Particle_telescope_t(SharedHistograms &hist, const size_t &num);
            void Fill(const subvector<Entry_t> &deltaE, const subvector<Entry_t> &E);
            void Fill(const std::vector<Entry_t> &deltaE, const std::vector<Entry_t> &E);

        private:
            SharedHistogram2Dp ede_spectra[NUM_SI_DE_TEL];
            SharedHistogram2Dp ede_spectra_raw[NUM_SI_DE_TEL];
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

            Particle_telescope_t particle_coincidence[NUM_SI_E_DET];

            Detector_Histograms_t *GetSpec(const DetectorType &type);

        public:
            HistManager(SharedHistograms &histograms, const UserConfiguration &configuration);
            ~HistManager() = default;

            //! Fill spectra with an event
            void AddEntry(Triggered_event &buffer);

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
            TEventQueue_t &input_queue;
            const UserConfiguration& configuration;


        public:
            CLASS_NAME(Sorter)
            Sorter(SharedHistograms& histograms, TEventQueue_t &input, const UserConfiguration& config);
            ~Sorter() override = default;
            void Run() override;
        };
    }
};

#endif // SORT_H