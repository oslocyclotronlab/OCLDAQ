
#include "sort_spectra.h"

#include <iostream>

#include <stdlib.h>
#include <strings.h>

int main()
{
    const int s = system("echo \"clear\" | nc -q 1 127.0.0.1 32010 >/dev/null 2>/dev/null");
    if( !WEXITSTATUS(s) ) {
        std::cout << "Sent 'clear' command to sort process." << std::endl;
        return EXIT_FAILURE;
    }

    // try to attach shared memory
    if( !spectra_attach_all(true) ) {
        std::cerr << "Failed to attach shm spectra." << std::endl;
        exit(EXIT_FAILURE);
    }
    // clear the spectra in shared memory
    for(int i=1; sort_spectra[i].name; ++i) {
        const sort_spectrum_t* s = &sort_spectra[i];
        bzero(s->ptr, s->ydim*s->xdim*sizeof(*s->ptr));
    }

    std::cout << "Cleared shared memory spectra (sort process not running)." << std::endl;

    spectra_detach_all();

    return EXIT_SUCCESS;
}


#if 0
// how to expand env variables:
#include <wordexp.h>

    for(int j=1; j<argc; ++j ) {
        wordexp_t p;
        wordexp(argv[j], &p, 0);
        char **w = p.we_wordv;
        for(unsigned int i=0; i < p.we_wordc; i++)
            printf("%s\n", w[i]);
        wordfree(&p);
    }
    exit(EXIT_SUCCESS);
#endif
