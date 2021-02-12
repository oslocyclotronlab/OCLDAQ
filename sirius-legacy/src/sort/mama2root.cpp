
#ifndef __CINT__
#include <stdio.h>
#include <stdlib.h>

#include <fstream>
#include <string>
#include <iostream>

#include <TFile.h>
#include <TH2.h>

#define STATIC static

using namespace std;
#endif

STATIC string substchars(string const& name)
{
    string n = name;
    if( n.size()>5 && n.find(".mama") == n.size()-5 )
        n.erase(n.size()-5, 5);
    for(unsigned i=0; i<n.size(); ++i) {
        char ch = n[i];
        if( !isalnum(ch) )
            n[i] = '_';
    }
    return n;
}

STATIC TH2F* errmsg(const char* msg)
{
    printf("error: %s", msg);
    return 0;
}

STATIC TH2F* mama2root(istream& mama, const string& name)
{
    string line;
    if( !getline(mama, line) || line.substr(0, 10) != "!FILE=Disk" )
        return errmsg("FILE");
    if( !getline(mama, line) || line.substr(0, 6) != "!KIND=" )
        return errmsg("KIND");
    if( !getline(mama, line) )
        return errmsg("LABORATORY");
    if( !getline(mama, line) )
        return errmsg("EXPERIMENT");

    if( !getline(mama, line) || line.substr(0, 9) != "!COMMENT=" )
        return errmsg("COMMENT");
    //printf("line = '%s'\n", line.c_str());
    string::size_type pos_b = line.find_first_not_of(" \t\n\r", 9);
    //printf("pos_b = %d\n", pos_b);
    if( pos_b == string::npos )
        pos_b = 9;
    //printf("pos_b = %d\n", pos_b);
    string::size_type pos_e = line.find_last_not_of (" \t\n\r")+1;
    //printf("pos_e = %d\n", pos_e);
    if( pos_e == string::npos )
        pos_e = 9;
    //printf("pos_e = %d\n", pos_e);
    string comment = line.substr(pos_b, pos_e - pos_b);
    //printf("comment = '%s'\n", comment.c_str());

    if( !getline(mama, line) ) // skip "TIME" line
        return errmsg("TIME");

    if( !getline(mama, line) || line.substr(0, 19) != "!CALIBRATION EkeV=6" )
        return errmsg("CALIBRATION");
    double y[3] = {0, 1, 0}, x[3] = {0, 1, 0};
    if( sscanf( line.c_str(), "!CALIBRATION EkeV=6,%lf,%lf,%lf,%lf,%lf,%lf",
                &x[0], &x[1], &x[2], &y[0], &y[1], &y[2] ) != 6 )
        return errmsg("CALIBRATION values");
    //printf("cal = x: %lf %lf %lf  y: %lf %lf %lf\n", x[0], x[1], x[2], y[0], y[1], y[2] );
    
    if( !getline(mama, line) ) // skip "PRECISION" line
        return errmsg("PRECISION");

    if( !getline(mama, line) || line.substr(0, 13) != "!DIMENSION=2," )
        return errmsg("DIMENSION");
    int nx=0, ny=0;
    if( sscanf( line.c_str(), "!DIMENSION=2,0:%d,0:%d", &nx, &ny ) != 2 )
        return errmsg("DIMENSION values");
    nx += 1;
    ny += 1;
    //printf("nx=%d ny=%d\n", nx, ny);

    if( !getline(mama, line) || line.substr(0, 9) != "!CHANNEL=" )
        return errmsg("CHANNEL");
    int cx=0, cy=0;
    if( sscanf( line.c_str(), "!CHANNEL=(0:%d,0:%d)", &cx, &cy ) != 2 )
        return errmsg("CHANNEL values");
    
    TH2F* m = new TH2F(name.c_str(), comment.c_str(), nx, x[0], nx*x[1]+x[0], ny, y[0], ny*y[1]+y[0]);
    m->SetOption( "colz" );
    m->SetContour( 64 );

    for(int iy=1; iy<=ny; ++iy) {
        for(int ix=1; ix<=nx; ++ix) {
            double d;
            mama >> d;
            m->SetBinContent(ix, iy, d);
        }
    }

    if( !getline(mama, line) ) // skip to end of line
        return errmsg("before IDEND");
    if( !getline(mama, line) || line.substr(0, 7) != "!IDEND=" )
        return errmsg("IDEND");
    if( !getline(mama, line) ) // skip empty line
        return errmsg("after IDEND");

    return m;
}

TH2F* mama2root(const string& filename, const string& name)
{
    ifstream mama(filename.c_str());
    if( !mama.is_open() )
        return 0;
    return mama2root(mama, substchars(name));
}

#ifndef __CINT__
#ifdef HAVE_MAIN
STATIC void help_exit(const char* progname)
{
    fprintf(stderr, "use: %s input.mama+ output.root\n"
            "   output filename (last parameter) must end with '.root'\n", progname);
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[])
{
    if( argc <= 2 )
        help_exit(argv[0]);
    std::string rootname = argv[argc-1];
    int rns = rootname.size();
    if( rns<6 || rootname.substr(rns-5) != ".root")
        help_exit(argv[0]);
    
    TFile *out = new TFile(argv[argc-1], "recreate");

    bool strip_dir = false;
    for(int a=1; a<argc-1; ++a) {
        const string arg = argv[a];
        if( arg == "--strip-dir" ) {
            strip_dir = true;
            continue;
        }
        TH2F* m;
        if( arg.size() > 0 && arg[0] == '-' )
            m = mama2root(cin, arg.substr(1).c_str());
        else {
            string::size_type idx = arg.find_last_of("/");
            string name = ( !strip_dir || idx == string::npos ) ? arg : arg.substr(idx+1);
            m = mama2root(arg, name);
        }
        if( !m )
            fprintf(stderr, "error reading '%s'\n", argv[a]);
    }

    out->Write();
    return EXIT_SUCCESS;
}
#endif
#endif

// g++ -Wall -W `root-config --cflags --libs` -DHAVE_MAIN -o /tmp/mama2root.exe mama2root.C
