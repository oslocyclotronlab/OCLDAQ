// -*- c++ -*-

#ifndef USERROOT_H
#define USERROOT_H 1

#include "user_routine.h"

#include <vector>
#include <string>
#include <map>

class TFile;

// ########################################################################

class UserROOT : public UserRoutine {
public:
    UserROOT();

    bool Init(bool online);
    bool Data(const std::string& filename);
    bool Cmd(const std::string& cmd);
    bool Finish() {  Write(); return true; }

    calibration_t& GetCalibration() { return calibration; }

protected:
    unsigned long time_start, time_now;
    void UnpackTime(unpacked_t* u);

    bool ReadNaIEnergyCalibration(std::istream& in);

    virtual void CreateSpectra() = 0;
    void Write();

    calibration_t calibration;
    bool have_gain;

    TFile* outfile;

    typedef std::vector<float> par_values_t;
    void PutParameter(const std::string& name, par_values_t& values);

    class Parameter {
        const par_values_t* values;
    public:
        Parameter() : values(0) { }
        Parameter(const par_values_t& v) : values(&v) { }
        float operator[](unsigned int idx) const
            { if(values && idx<values->size()) return (*values)[idx]; else return 0; }
    };

    Parameter GetParameter(const std::string& name);

private:
    typedef std::map<std::string, par_values_t> parameters_t;
    parameters_t parameters;
};

// ########################################################################

#endif /* USERROOT_H */
