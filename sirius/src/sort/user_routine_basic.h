// -*- c++ -*-

#ifndef USER_ROUTINE_BASIC_H
#define USER_ROUTINE_BASIC_H 1

#include "user_routine.h"
#include "sort_calib.h"

class UserRoutineBasic : public UserRoutine {
public:
    UserRoutineBasic();

    bool Init(bool online);
    bool Data(const std::string& filename);
    bool Cmd(const std::string& cmd);
    bool Finish();

    calibration_t& GetCalibration() { return calibration; }

protected:
    calibration_t calibration;
    bool have_gain, have_telewin, have_range;
};

#endif /* USER_ROUTINE_BASIC_H */
