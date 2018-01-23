
#include "utilities.h"
#include "user_routine.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include <zlib.h>

#if _FILE_OFFSET_BITS != 64
#error must compile with _FILE_OFFSET_BITS == 64
#endif

char leaveprog='n'; // leave program (Ctrl-C / INT)

const unsigned int datalen = 0x8000;
unsigned int data[datalen];
const unsigned int datalen_char = datalen*sizeof(data[0]);

bool is_tty = false;

static UserRoutine* user_routine;

/// Create a UserRoutine object to be used in the sorting routine.

/// This function is to be linked in from the sorting code, like, e.g.,
/// from user_routine_root.cpp.
extern UserRoutine* user_routine_create();

/// \file
/// The offline sorting program reads a batch file where data files,
/// gain-shifts etc. are specified. This file may contain lines like
/// this:
/// 
/// - data file /path/to/file -- this will try to read the complete file
///
/// - data buffers 10 20 file /path/to/file -- will try to read file,
///   but only buffers 10 to 19 (put -1 as end to read to the end)
///
/// - gain file /path/to/gainshiftfile -- load gain and shift from the file
/// 
/// - gain data 1 1 1 ... -- read the gain and shift directly; the
///   format is the same as in the gain-shift-file, but everything
///   must be on one line
/// 
/// - quit -- stop sorting
/// 
/// - dump -- dump all spectra
/// 
/// - range /path/to/file -- read range data from file
/// 
/// - telewin file /path/to/file -- read the telewin data from the file
/// 
/// - telewin data 0 511 0 511 ... -- read the telewin data directly;
///   the format is the same as in the telewin-file, but everything
///   must be on one line
/// 
/// When the program exits (normally) it dumps all spectra.
/// 
/// Comment lines, i.e. lines with # as first character, are ignored.

// ######################################################################## 

/// Signal handler for Ctrl-C.
void keyb_int(int sig_num)
{
    if (sig_num == SIGINT) {
        printf("\n\nLeaving...\n");
        leaveprog = 'y';
    }
}

// ########################################################################

//#define USE_ZLIB
/// Class used to read buffer files.

/// Using this class allows to move some file-reading code here and to
/// switch the support for reading compressed files relatively easy.
class buffer_file {
public:
    /// Open a buffer file.
    
    /// Does not complain in case of error. The error condition can be checked
    /// using the method okay().
    buffer_file(const std::string& filename);

    ~buffer_file();

    /// Read one buffer.

    /// The buffer is stored in the given memory region which has to be big enough
    /// for the buffer, i.e. datalen bytes.
    /// @return 0 for EOF, <0 for errors, >0 if all went okay
    int read_buffer(char* data);

    /// Seek in a buffer.

    /// Only allows seeking forward.
    /// @return true if the seek operation was successful
    bool seek_buffer(int buf);


    /// Check if there was an error reading or opening the file.
    bool okay() const { return file != 0; }

private:

    /// Close the file.
    void close();

#ifdef USE_ZLIB
    gzFile file;
#else
    FILE* file;
#endif
};

buffer_file::buffer_file(const std::string& filename)
    : file(0)
{
#ifdef USE_ZLIB
    file = gzopen(filename.c_str(), "rb");
#else
    file = fopen(filename.c_str(), "rb");
#endif
}

buffer_file::~buffer_file()
{
    close();
}

void buffer_file::close()
{
    if( file ) {
#ifdef USE_ZLIB
        gzclose(file);
#else
        fclose(file);
#endif
    }
}

int buffer_file::read_buffer(char* data)
{
    if( !file )
        return -1;
    unsigned int have = 0;
    while( have<datalen_char ) {
#ifdef USE_ZLIB
        int now = gzread(file, data+have, datalen_char-have);
#else
        int now = fread(data+have, 1, datalen_char-have, file);
#endif
        if( now<=0 ) {
            close();
            return now;
        } else {
            have += now;
        }
    }
    return datalen_char;
}

bool buffer_file::seek_buffer(int buf)
{
    if( file ) {
        int want = buf*datalen_char;
#ifdef USE_ZLIB
        return gzseek(file, want, SEEK_SET) == want;
#else
        return fseek(file, want, SEEK_SET) == want;
#endif
    } else {
        return false;
    }
}

// ########################################################################

static bool data_command(std::istream& icmd)
{
    int buf_start=0, buf_end=-1;

    std::string tmp;
    icmd >> tmp;
    if( tmp == "buffers" )
        icmd >> buf_start >> buf_end >> tmp;

    if( tmp == "file" ) {
        std::string filename;
        icmd >> filename; // XXX no spaces possible

        user_routine->Data(filename);

        // open data file
        buffer_file df(filename);
        if( !df.okay() ) {
            std::cerr << "data: could not open '" << filename << "'." << std::endl;
            return false;
        }
        std::cout << "data: opened file '" << filename << "'." << std::endl;

        std::string buf_end_txt = buf_end > buf_start ? ioprintf("%d", buf_end) : std::string("end");
        std::cout << "data: shall read buffers " << buf_start << "..." << buf_end_txt << std::endl;
        int buffer_count = 0, bad_buffer_count = 0;

        df.seek_buffer(buf_start);
        
        // remember starting time
        timeval last_time;
        gettimeofday(&last_time, 0);
        int delta = (is_tty) ? 10 : 50;
        int print_buffer = buf_start + delta;
        
        unpacked_t unpacked;
        
        // loop over all buffers
        for(int b=buf_start; leaveprog=='n' && (buf_end<buf_start || b<buf_end); ++b) {

            // read one buffer from the file
            int r = df.read_buffer((char*)data);
            if( r==0 ) {
                // end of file
                break;
            } else if( r<0 ) {
                std::cerr << "\ndata: error reading buffer " << b << std::endl;
                return false;
            }
            
            if( !user_routine->Buffer(b) )
                return false;
        
            // prepare unpacking procedures for next buffer
            unpack_next_buffer();
            
            // unpack and sort all events
            int unpack_err=0;
            while(leaveprog=='n') {
                unpack_err = unpack_next_event(data, datalen, &unpacked);
                if( unpack_err )
                    break;
                user_routine->Sort(&unpacked);
            }
            
            buffer_count += 1;
            if( unpack_err > 1 )
                bad_buffer_count += 1;
                    
            // from time to time, print a message
            if( b == print_buffer ) {
                if( is_tty ) {
                    std::cout << '\r' << buffer_count << '/' << bad_buffer_count
                              <<' '<< unpack_get_avelen() << ' ' << std::flush;
                    
                    timeval now_time;
                    gettimeofday(&now_time, 0);
                    
                    const double l = last_time.tv_sec + last_time.tv_usec*1e-6;
                    const double n =  now_time.tv_sec +  now_time.tv_usec*1e-6;
                    const double bufs_per_sec = delta / (n-l);

                    std::cout << bufs_per_sec << " bufs/s " << std::flush;

                    delta = (int)std::max(1.0, bufs_per_sec);
                    last_time = now_time;
                } else {
                    std::cout << '.' << std::flush;
                }
                print_buffer += delta;
            }
        }
        std::cout << '\r' << buffer_count << '/' << bad_buffer_count
                  <<' '<< unpack_get_avelen() << ' ' << std::endl;
    } else {
        std::cerr << "data: Expected data [buffers <from> <to>] file <filename>.\n";
        return false;
    }
    return true;
}

// ########################################################################

static bool next_command(const std::string& cmd)
{
    std::istringstream icmd(cmd.c_str());

    std::string name, tmp;
    icmd >> name;

    if( name == "quit") {
        leaveprog = 'y';
        return true;
    } else if( name == "data" ) {
        return data_command(icmd);
    } else {
        return user_routine->Cmd(cmd);
    }
}

// ########################################################################

bool next_commandline(std::istream& in, std::string& cmd_line)
{
    cmd_line = "";
    std::string line;
    while( getline(in, line) ) {
        int ls = line.size();
        if( ls==0 ) {
            break;
        } else if( line[ls-1] != '\\' ) {
            cmd_line += line;
            break;
        } else {
            cmd_line += line.substr(0, ls-1);
        }
    }
    return in || !cmd_line.empty();
}

// ########################################################################

int main(int argc, char* argv[])
{
    if( argc != 2 ) {
        std::cerr << "offline_sort batchfile" << std::endl;
        exit(EXIT_FAILURE);
    }

    is_tty = isatty(STDOUT_FILENO);

    signal(SIGINT, keyb_int); // set up interrupt handler (Ctrl-C)
    signal(SIGPIPE, SIG_IGN);

    user_routine = user_routine_create();
    if( !user_routine->Init(false) )
        exit(EXIT_FAILURE);

    std::cout << "user routine: " << user_routine->GetId() << '\n';

    std::ifstream batch_file(argv[1]);
    std::string batch_line;
    while( leaveprog=='n' && next_commandline(batch_file, batch_line) ) {
        if( batch_line.size()==0 || batch_line[0] == '#' )
            continue;
        if( !next_command(batch_line) )
            break;
    }

    user_routine->Finish();
    delete user_routine;

    return 0;
}

/* for emacs */
/*** Local Variables: ***/
/*** c-basic-offset:4 ***/
/*** indent-tabs-mode: nil ***/
/*** compile-command:"make" ***/
/*** End: ***/
