
#include "spectrum_rw.h"

#include "utilities.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <time.h>

#define NDEBUG
#include "debug.h"

/**
 * Encode the current date and time like '22-Mar-94 HH:MM:SS'
 *
 * @param out   the date text is stored here, must have at least size characters
 * @param size  the size available in the output string
 */
static std::string datetime()
{
    char tmp[64];
    time_t now = time(0);
    int s = strftime(tmp, sizeof(tmp), "%d-%b-%y %H:%M:%S", localtime(&now));
    tmp[s] = '\0';
    return tmp;
}

/****************************************************************************/

/**
 * Write spectrum or matrix header.
 *
 * @param fp       output file, must be open already
 * @param comment  comment for spectrum/matrix
 * @param xdim     x dimension for spectrum/matrix
 * @param ydim     <0 for spectrum, y dimension for matrix
 * @param cal      calibration coefficients
 */
static void spectrum_write_header(std::ostream& fp, const std::string& comment,
                                  int xdim, int ydim, float *cal)
{
    if(cal[0] + cal[1] + cal[2] == 0) {
        cal[0] = 0.;
        cal[1] = 1.;
        cal[2] = 0.;
    }
    if(ydim<0 && cal[3] + cal[4] + cal[5] == 0) {
        cal[3] = 0.;
        cal[4] = 1.;
        cal[5] = 0.;
    }

    const int ical = (ydim<0) ? 3 : 6;

    fp << "!FILE=Disk \n"
       << "!KIND=" << ( (ydim<0) ? "Spectrum" : "Matrix" ) << " \n"
       << "!LABORATORY=Oslo Cyclotron Laboratory (OCL) \n"
       << "!EXPERIMENT=Sirius \n"
       << "!COMMENT=" << comment << '\n'
       << "!TIME=" << datetime() << '\n'
       << "!CALIBRATION EkeV=" << ical;
    for(int i=0; i<ical; i++)
        fp << ioprintf(",%13.6E", cal[i]);
    fp << "\n"
       << "!PRECISION=16\n";
    if( ydim<0 ) {
        fp << "!DIMENSION=1,0:" << ioprintf("%4d", xdim-1) << '\n'
           << "!CHANNEL=(0:" << ioprintf("%4d", xdim-1) << ")\n";
    } else {
        fp << ioprintf("!DIMENSION=2,0:%4d,0:%4d\n", xdim-1, ydim-1)
           << ioprintf("!CHANNEL=(0:%4d,0:%4d)\n",   xdim-1, ydim-1);
    }
}

/****************************************************************************/

/**
 * Write 1D spectrum to disk.
 *
 * @param fp       output file, must be open already
 * @param comment  comment for spectrum
 * @param pxdim    pointer to x dimension for spectrum
 * @param ax       array of 8192 channel values
 * @param cal      calibration coefficients
 */
int norw1dim(std::ostream& fp, const std::string& comment, const int& xdim,
             const float* ax, float cal[6])
{
    spectrum_write_header(fp, comment, xdim, -1, cal);
    for(int i=0; i<xdim; i++)
        //fp << ioprintf("%e ", (double)ax[i]);
        fp << ax[i] << ' ';
    fp << "\n!IDEND=\n\n" << std::flush;

    return ( !fp ) ? -1 : 0;
}

/****************************************************************************/

/**
 * Write 2D matrix to disk.
 *
 * @param fp       output file, must be open already
 * @param comment  comment for matrix
 * @param xdim     x dimension of matrix
 * @param ydim     y dimension of matrix
 * @param mx       array of 4096*512 channel values
 * @param cal      calibration coefficients
 */
int norw2dim(std::ostream& fp, const std::string& comment, const int xdim, const int ydim,
             const float* mx, float cal[6])
{
    spectrum_write_header(fp, comment, xdim, ydim, cal);
    for(int j=0; j<ydim; j++) {
        for(int i=0; i<xdim; i++)
            //fp << ioprintf("%e ", (double)mx[i + j*xdim]);
            fp << mx[i + j*xdim] << ' ';
        fp << '\n';
    }
    fp << "!IDEND=\n\n" << std::flush;
    
    return 0;
}

/****************************************************************************/

/**
 * Write 2D matrix to disk.
 *
 * @param fp       output file, must be open already
 * @param comment  comment for matrix
 * @param xdim     x dimension of matrix
 * @param ydim     y dimension of matrix
 * @param mx       array of 4096*512 channel values
 * @param cal      calibration coefficients
 */
int norw2dim(std::ostream& fp, const std::string& comment, const int xdim, const int ydim,
             const int* mx, float cal[6])
{
    spectrum_write_header(fp, comment, xdim, ydim, cal);
    for(int j=0; j<ydim; j++) {
        for(int i=0; i<xdim; i++)
            //fp << ioprintf("%e ", (double)mx[i + j*xdim]);
            fp << mx[i + j*xdim] << ' ';
        fp << '\n';
    }
    fp << "!IDEND=\n\n" << std::flush;
    
    return 0;
}

/****************************************************************************/

/**
 * Read spectrum or matrix header.
 *
 * @param fp            output file, must be open already
 * @param comment       comment for spectrum/matrix
 * @param comment_size  maximal comment size
 * @param pxdim         x dimension for spectrum/matrix
 * @param pydim         0 for spectrum, y dimension for matrix
 * @param cal           calibration coefficients
 */
static void spectrum_read_header(std::istream& fp, std::string& comment,
                                 int& xdim, int& ydim, float* cal)
{
    DBGV(!fp);
    fp.ignore(128, '\n'); // skip '!FILE=Disk'
    fp.ignore(128, '\n'); // skip '!KIND=...'
    fp.ignore(128, '\n'); // skip '!LABORATORY=...'
    fp.ignore(128, '\n'); // skip '!EXPERIMENT=...'

    fp.ignore(9);         // read '!COMMENT='
    std::getline(fp, comment);
    DBGV(comment);

    fp.ignore(128, '\n'); // skip '!TIME=...'

    DBGL;
    // read calibration
    {   std::string line;
        getline(fp, line);
        std::istringstream ifp(line.c_str());

        ifp.ignore(sizeof("!CALIBRATION EkeV=")-1);
        int ical = -1;
        ifp >> ical;
        DBGV(ical);
        DBGV(ifp.tellg());
        if( ical!=3 && ical!=6 )
            return;
        for(int i=0; i<ical; ++i) {
            ifp.ignore(1); // skip ','
            ifp >> cal[i];
            DBGV(cal[i]);
            DBGV(ifp.tellg());
        }
    }

    fp.ignore(128, '\n'); // skip '!PRECISION=...'

    // read DIMENSION
    {   std::string line;
        getline(fp, line);
        std::istringstream ifp(line.c_str());
        ifp.ignore(15) >> xdim;
        if( ydim>=0 ) {
            ifp.ignore(3) >> ydim;
            ydim += 1;
            DBGV(ydim);
        }
        xdim += 1;
        DBGV(xdim);
    }
    
    fp.ignore(128, '\n'); // skip '!CHANNEL=...'
}

/****************************************************************************/

/**
 * Read 1D spectrum from disk.
 *
 * @param fp       output file, must be open already
 * @param comment  comment for spectrum
 * @param pxdim    pointer to x dimension for spectrum
 * @param ax       array of 8192 channel values
 * @param cal      calibration coefficients
 */
int norr1dim(std::istream& fp, std::string& comment, int& xdim, float* ax, float cal[6])
{
    int ydim = -1;
    spectrum_read_header(fp, comment, xdim, ydim, cal);

    for(int i=0; i<xdim; i++)
        fp >> ax[i];
    return (!fp) ? -1 : 0;
}

/****************************************************************************/

/**
 * Read 2D matrix from disk.
 *
 * @param fp       output file, must be open already
 * @param comment  comment for matrix
 * @param pxdim    pointer to x dimension for matrix
 * @param pydim    pointer to y dimension for matrix
 * @param mx       array of 4096*512 channel values
 * @param cal      calibration coefficients
 */
int norr2dim(std::istream& fp, std::string& comment, int& xdim, int& ydim,
             float* mx, float cal[6])
{
    spectrum_read_header(fp, comment, xdim, ydim, cal);
    
    for(int j=0; j<ydim; j++)
        for(int i=0; i<xdim; i++)
            fp >> mx[i + j*xdim];

    return (!fp) ? -1 : 0;
}

/****************************************************************************/

/**
 * Read 2D matrix from disk.
 *
 * @param fp       output file, must be open already
 * @param comment  comment for matrix
 * @param pxdim    pointer to x dimension for matrix
 * @param pydim    pointer to y dimension for matrix
 * @param mx       array of 4096*512 channel values
 * @param cal      calibration coefficients
 */
int norr2dim(std::istream& fp, std::string& comment, int& xdim, int& ydim,
             int* mx, float cal[6])
{
    spectrum_read_header(fp, comment, xdim, ydim, cal);
    
    for(int j=0; j<ydim; j++)
        for(int i=0; i<xdim; i++) {
            float tmp=0;
            fp >> tmp;
            mx[i + j*xdim] = (int)tmp;
        }

    return (!fp) ? -1 : 0;
}

// ########################################################################

bool dump_spectrum(const sort_spectrum_t* s, float* cal, const char* filename)
{
    if( !s )
        return false;

    if( !filename )
        filename = s->name;
    std::ofstream out(filename);
    float cal_dfl[6] = { 0, 1, 0, 0, 1, 0 };
    if( !cal )
        cal = cal_dfl;
    norw2dim(out, s->description, s->xdim, s->ydim, s->ptr, cal);

    return !(!out);
}

// ########################################################################
// ########################################################################
// ########################################################################

#ifdef TEST_READ

#include <fstream>
#include <stdlib.h>

static float mx[4096][512];
static float ax[4096];

int main(int argc, char* argv[])
{
    DBGL;
    if( argc != 3 ) {
        std::cerr << "test_rw 1/2 mama-spectrum/matrix" << std::endl;
        exit(EXIT_FAILURE);
    }

    DBGL;
    std::ifstream in(argv[2]);

    float cal[6];
    std::string comment;
    int xdim = 0, ydim = 0, err=-1;
    if( std::string(argv[1])=="1" ) {
        DBGL;
        err = norr1dim(in, comment, xdim, ax, cal);
    } else {
        DBGL;
        err = norr2dim(in, comment, xdim, ydim, mx, cal);
    }
    if( err==0 ) {
        std::cout << "comment = '" << comment << "'\n"
                  << "cal = ";
        for(int i=0; i<6; ++i)
            std::cout << ' ' << cal[i];
        std::cout << "\nxdim = " << xdim << " ydim = " << ydim << std::endl;
        return EXIT_SUCCESS;
    } else {
        std::cerr << "read error" << std::endl;
        return EXIT_FAILURE;
    }
}

#endif /* TEST_READ */
