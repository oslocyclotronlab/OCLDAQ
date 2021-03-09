
#include <stdlib.h>

#include "acq_log.h"
#include "m_sort.h"

void acq_dump()
{
    if( !m_sort_dump() )
        log_message(LOG_ERR, "Could not dump spectra.\n");
}
