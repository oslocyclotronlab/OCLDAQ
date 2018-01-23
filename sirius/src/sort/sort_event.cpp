
#include "sort_event.h"

#include <cmath>
#include <string.h>

using namespace std;

static const char negative = 0xCC;

// ----------------------------------------------------------------------------
TEvent::TEvent(bool create)
{
    if( create ) {
        na_i  = new short[32];
        na_e  = new float[32];
        na_t  = new float[32];

        ge_i  = new short[6];
        ge_e  = new float[6];
        ge_t  = new float[6];

        si_e_i = new short[8];
        si_e_e = new float[8];
        si_e_g = new float[8];

        si_de_i = new short[64];
        si_de_e = new float[64];
    } else {
        na_i  = 0;
        na_e  = 0;
        na_t  = 0;

        ge_i  = 0;
        ge_e  = 0;
        ge_t  = 0;

        si_e_i = 0;
        si_e_e = 0;
        si_e_g = 0;

        si_de_i = 0;
        si_de_e = 0;
    }
    Clear();
}

// ----------------------------------------------------------------------------
TEvent::~TEvent()
{
    delete[] na_i;
    delete[] na_e;
    delete[] na_t;

    delete[] ge_i;
    delete[] ge_e;
    delete[] ge_t;

    delete si_e_i;
    delete si_e_e;
    delete si_e_g;
    delete si_de_i;
    delete si_de_e;
}

// ----------------------------------------------------------------------------
void TEvent::Clear(const Option_t*)
{
    na_count = 0;
    ge_count = 0;
    si_e_count = 0;
    si_de_count = 0;

    char* first = reinterpret_cast<char*>( &memset_begin );
    char* last  = reinterpret_cast<char*>( &memset_end );
    memset( first, negative, (last-first) );
}

ClassImp(TEvent);
