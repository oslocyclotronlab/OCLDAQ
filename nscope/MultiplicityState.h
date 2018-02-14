
#ifndef MULTIPLICITYSTATE_H
#define MULTIPLICITYSTATE_H 1

#include <stdint.h>
#include <bitset>
#include <iostream>
#include <TString.h> 

class MultiplicityState 
{
    private:
    uint32_t fMultMaskLow;
    uint32_t fMultMaskHigh;
    double   fCoincTime;

    public:
    MultiplicityState();
    MultiplicityState(const uint32_t lowmask,
            const uint32_t highmask, 
            const double coinctime);

    MultiplicityState(const MultiplicityState& state);
    MultiplicityState& operator=(const MultiplicityState& state);

    bool IsValid() const;
    
    uint32_t GetSelfChannelMask() const;
    uint32_t GetRightChannelMask() const;
    uint32_t GetLeftChannelMask() const;

    uint32_t GetSelfMultiplicityThreshold() const;
    uint32_t GetRightMultiplicityThreshold() const;
    uint32_t GetLeftMultiplicityThreshold() const;

    uint32_t GetMultiplicityMaskLow() const {return fMultMaskLow;}
    uint32_t GetMultiplicityMaskHigh() const {return fMultMaskHigh;}

    double GetCoincidenceTime() const { return fCoincTime;}

    void Print() const;

};

inline bool operator==(MultiplicityState& lhs, MultiplicityState& rhs)
{
    bool flag = true;

    flag = flag && (lhs.GetMultiplicityMaskLow() == rhs.GetMultiplicityMaskLow());
    flag = flag && (lhs.GetMultiplicityMaskHigh() == rhs.GetMultiplicityMaskHigh());
    flag = flag && (lhs.GetCoincidenceTime() == rhs.GetCoincidenceTime());

    return flag;

};

inline bool operator!=(MultiplicityState& lhs, MultiplicityState& rhs)
{
    return !(lhs==rhs);
}

#endif
