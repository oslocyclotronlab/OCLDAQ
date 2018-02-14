#include "MultiplicityState.h"

MultiplicityState::MultiplicityState()
 : fMultMaskLow(0x0), fMultMaskHigh(0x0), fCoincTime(0.0)
{}

MultiplicityState::MultiplicityState(const uint32_t lowmask, const uint32_t highmask, const double coinctime)
: fMultMaskLow(lowmask), fMultMaskHigh(highmask), fCoincTime(coinctime)
{}

MultiplicityState::MultiplicityState(const MultiplicityState& state)
    : fMultMaskLow(state.fMultMaskLow), fMultMaskHigh(state.fMultMaskHigh),
    fCoincTime(state.fCoincTime)
{} 

MultiplicityState& MultiplicityState::operator=(const MultiplicityState& state)
{
    if (this!=&state) {
        fMultMaskLow = state.fMultMaskLow;
        fMultMaskHigh = state.fMultMaskHigh;
        fCoincTime = state.fCoincTime;
    }

    return *this;
}

bool MultiplicityState::IsValid() const
{
    return (fCoincTime>=0);
}

uint32_t MultiplicityState::GetSelfChannelMask() const
{
    return (fMultMaskLow & 0xFFFF);
}

uint32_t MultiplicityState::GetRightChannelMask() const
{
    return ((fMultMaskLow>>16) & 0xFFFF);
}

uint32_t MultiplicityState::GetLeftChannelMask() const
{
    return (fMultMaskHigh & 0xFFFF);
}

uint32_t MultiplicityState::GetSelfMultiplicityThreshold() const 
{
    return ((fMultMaskHigh>>22) & 0x7);
}

uint32_t MultiplicityState::GetRightMultiplicityThreshold() const 
{
    return ((fMultMaskHigh>>25) & 0x7);
}

uint32_t MultiplicityState::GetLeftMultiplicityThreshold() const 
{
    return ((fMultMaskHigh>>28) & 0x7);
}

void MultiplicityState::Print() const 
{
    std::cout << Form("Low mask = 0x%0x  Upper mask = 0x%0x", fMultMaskLow, fMultMaskHigh) << std::endl;
    std::cout << Form("Coincidence time = %6.2f us",fCoincTime) << std::endl;
    std::cout << "Self channel mask         : " << std::bitset<16>(GetSelfChannelMask()) << std::endl;
    std::cout << "Right channel mask        : " << std::bitset<16>(GetRightChannelMask()) << std::endl;
    std::cout << "Left channel mask         : " << std::bitset<16>(GetLeftChannelMask()) << std::endl;
    std::cout << "Self multiplicity thresh  : " << GetSelfMultiplicityThreshold()  << std::endl;
    std::cout << "Right multiplicity thresh : " << GetRightMultiplicityThreshold() << std::endl;
    std::cout << "Left multiplicity thresh  : " << GetLeftMultiplicityThreshold()  << std::endl;
}
