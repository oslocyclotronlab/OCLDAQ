// -*-c++-*-

#ifndef SORT_EVENT_H
#define SORT_EVENT_H

#include <TObject.h>

class TEvent : public TObject
{
public:
    int   na_count;                // NaI count
    short *na_i; //[na_count]         NaI id
    float *na_e;  //[na_count]        NaI energy
    float *na_t;  //[na_count]        NaI time

    // ge data
    int ge_count;                  // germanium count
    short *ge_i; //[ge_count]         germanium id
    float *ge_e; //[ge_count]         germanium energy
    float *ge_t; //[ge_count]         germanium time

    int  si_e_count;
    short* si_e_i; // [si_e_count]     Si detector number
    float* si_e_e; // [si_e_count]     Si E
    float* si_e_g; // [si_e_count]     Si E guard ring

    int  si_de_count;
    short* si_de_i; // [si_de_count]   Si front pad id (8*back+front)
    float* si_de_e; // [si_de_count]   Si dE

    // WARNING: everything from memset_begin to memset_end will be memset to 0
    short memset_begin; //!

    unsigned short trigger_pattern; // trigger pattern from the TPU
    unsigned long long event_n;    // event number, must be 64bit!
    short event_l;                 // event length

    unsigned long timestamp;   //!
    unsigned long scalers[16]; //!

    short memset_end; //!

    TEvent(bool create=false);
    ~TEvent();

    void Clear(const Option_t* option=0);

    ClassDef(TEvent,1);
};

#endif // SORT_EVENT_H
