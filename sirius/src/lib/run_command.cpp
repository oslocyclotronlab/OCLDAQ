
#include "run_command.h"

#include "utilities.h"

#include <cstdlib>
#include <fstream>
#include <sstream>

#include <string.h>
#include <sys/wait.h>
#include <wordexp.h>

#include <sys/types.h>
#include <unistd.h>

#define NDEBUG
#include "debug.h"

// ########################################################################

std::vector<std::string> word_expand(const char* cmd)
{
    std::vector<std::string> expanded;

    wordexp_t p;
    ::wordexp(cmd, &p, 0);
    char **w = p.we_wordv;
    for(unsigned int i=0; i < p.we_wordc; i++)
        expanded.push_back(w[i]);
    ::wordfree(&p);

    return expanded;
}

// ########################################################################

bool run_background(const std::string& cmd, const std::vector<std::string>& xargs)
{
    pid_t pid = ::fork();
    if( pid == 0 ) {
        // child process

        // expand environment etc. into single strings
        std::vector<std::string> expanded = word_expand(cmd.c_str());
        const unsigned es = expanded.size();
        
        // copy to a string array
        const unsigned NARGV = 128;
        char* argv[NARGV];
        if( es==0 || es + xargs.size() > NARGV-1 )
            ::exit(EXIT_FAILURE);
        unsigned int i;
        for( i=0; i<es; ++i )
            argv[i] = ::strdup(expanded[i].c_str());
        for(unsigned int x=0; x<xargs.size(); ++x )
            argv[i++] = ::strdup(xargs[x].c_str());
        argv[i] = 0;

#if !defined(NDEBUG)
        DBGV(i);
        for(unsigned int j=0; j<i; ++j)
            std::cout << "argv[" << j << "] = '" << argv[j] << "'\n";
#endif

        // execute program with arguments
        if( ::execvp(argv[0], argv) < 0 )
            // reached in case of execvp failure
            ::exit(EXIT_FAILURE);

        return false; // not reached, but compiler does not know
    } else if( pid != -1 ) {
        // parent process

        // wait a little and check child process status
        ::usleep(250000);
        int status=0;
        pid_t p = ::waitpid(pid, &status, WNOHANG);
        if( p == -1 ) {
            // waitpid error
            return false;
        } else if( p == 0 ) {
            // child still running
            return true;
        } else if( WIFEXITED(status)) {
            // child exited, return true if exit status is 0
            return WEXITSTATUS(status)==0;
        } else if( WIFSIGNALED(status) || WCOREDUMP(status) || WIFSTOPPED(status) ) {
            // child was signaled, dumped core, ...
            return false;
        } else {
            // child still running
            return true;
        }
    } else { // fork() error
        return false;
    }
}

// ########################################################################

bool run_background(const std::string& cmd)
{
    const std::vector<std::string> xargs;
    return run_background(cmd, xargs);
}

// ########################################################################
// ########################################################################

command_list::command_list()
{
}

// ########################################################################

command_list::~command_list()
{
}

// ########################################################################

bool command_list::read(std::istream& infile)
{
    std::map<std::string, std::string> nc;

    std::string line;
    while( std::getline(infile, line) ) {
        // ignore comments and empty lines
        if( line.empty() || line[0] == '#' )
            continue;

        // search for "=" sign on each line
        std::string::size_type pos_eq = line.find("=");

        // not found => error
        if( pos_eq == std::string::npos )
            return false;

        // extract parts before and after "=", and remove blanks etc around
        std::string key = strip(line.substr(0, pos_eq));
        std::string val = strip(line.substr(pos_eq+1));

        // if the key exists already => error
        if( nc.find(key) != nc.end() )
            return false;
        nc[key] = val;
    }
    commands.swap(nc);
    return true;
}

// ########################################################################

bool command_list::read(const char* infilename)
{
    std::ifstream infile(infilename);
    if( !infile )
	return false;
    return read(infile);
}

// ########################################################################

bool command_list::read_text(const char* text)
{
    std::istringstream intext(text);
    return read(intext);
}

// ########################################################################

bool command_list::run(const std::string& cmd_key, const std::vector<std::string>& xargs)
{
    if( commands.find(cmd_key) == commands.end() )
        return false;

    const std::string& cmd = commands[cmd_key];
    if( cmd.empty() )
        return false;

    return run_background(cmd, xargs);
}

// ########################################################################
// ########################################################################

#ifdef TEST_RUN_COMMAND

#include <iostream>

int main(int argc, char* argv[])
{
    if( argc < 2 ) {
        std::cerr << argv[0] << " <infile> <key>*" << std::endl;
        exit(EXIT_FAILURE);
    }

    command_list c;
    c.read_text("mama = xterm -bg white -fg blue -title mama -e 'echo this should be mama; read stdin'\n"
                "papa = echo 'papa'");

    if( !c.read(argv[1]) )
        std::cerr << "Could not read '" << argv[1] << "', using defaults." << std::endl;

    std::cout << "\n\n";

    for(int i=2; i<argc; ++i) {
        if( !c.run(argv[i]) )
            std::cerr << "problem running '" << argv[i] << '\'' << std::endl;
    }
    
    return EXIT_SUCCESS;
}

#endif /* TEST_RUN_COMMAND */
