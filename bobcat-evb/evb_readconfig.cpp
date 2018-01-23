
#include "evb_readconfig.h"

#include <fstream>
#include <stdio.h>

// ########################################################################

bool read_config_values(const char* filename, unsigned short* values, int count)
{
    std::ifstream in(filename);
    printf("Reading %d value(s) from '%s': ", count, filename);
    fflush(stdout);
    for(int i=0; in && i<count; ++i) {
        in >> values[i];
        printf("%d ", values[i]);
        fflush(stdout);
    }
    printf(" DONE\n");
    return in;
}

