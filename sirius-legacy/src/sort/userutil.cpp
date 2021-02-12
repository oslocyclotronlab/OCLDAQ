
#include "userutil.h"

#include "sort_format.h"

std::string fmt(const char *fmt, ...)
{
    char msgbuf[8192];
    va_list ap;
    va_start (ap, fmt);
    (void) vsnprintf(msgbuf, sizeof(msgbuf), fmt, ap);
    va_end (ap);

    return msgbuf;
}

// ########################################################################

TH1* Spec( const std::string& name, const std::string& title,
           int channels, double left, double right, const std::string& xtitle )
{
    TH1* h = new TH1I( name.c_str(), title.c_str(), channels, left, right );

    TAxis* xax = h->GetXaxis();
    xax->SetTitle(xtitle.c_str());
    xax->SetTitleSize(0.03);
    xax->SetLabelSize(0.03);

    TAxis* yax = h->GetYaxis();
    yax->SetLabelSize(0.03);

    return h;
}

// ########################################################################

TH2* Mat( const std::string& name, const std::string& title,
          int ch1, double l1, double r1, const std::string& xtitle, 
          int ch2, double l2, double r2, const std::string& ytitle)
{
    TH2* mat = new TH2F( name.c_str(), title.c_str(), ch1, l1, r1, ch2, l2, r2 );
    mat->SetOption( "colz" );
    mat->SetContour( 64 );

    TAxis* xax = mat->GetXaxis();
    xax->SetTitle(xtitle.c_str());
    xax->SetTitleSize(0.03);
    xax->SetLabelSize(0.03);

    TAxis* yax = mat->GetYaxis();
    yax->SetTitle(ytitle.c_str());
    yax->SetTitleSize(0.03);
    yax->SetLabelSize(0.03);
    yax->SetTitleOffset(1.3);

    TAxis* zax = mat->GetZaxis();
    zax->SetLabelSize(0.025);

    return mat;
}

// ########################################################################

int unpack_nai(unpacked_t* u, nai_data_t* nai, int* idx)
{
    // unpack NaI time and energy into an array so that time and
    // energy are 'together'
    int na_n = 0;
    for(int i=0; i<32; ++i)
        idx[i] = -1;
    for( int i=0; i<=u->nanu; i++ ) {
        int id = u->nai[i];
        if( id<0 || id>=28 )
            continue;
        idx[id] = na_n;
        nai[na_n].id = id;
        nai[na_n].e  = u->na[i];
        nai[na_n].t  = 0;
        na_n += 1;
    }
    for( int i=0; i<=u->tnanu; i++ ) {
        int id = u->tnai[i];
        if( id<0 || id>=28 )
            continue;
        if( idx[id]<0 ) {
            idx[id] = na_n;
            nai[na_n].id = id;
            nai[na_n].e  = 0;
            na_n += 1;
        }
        nai[idx[id]].t = u->tna[i];
    }
    return na_n;
}

