
#include <stdlib.h>

#include "acq_log.h"
#include "m_sort.h"

void acq_clear()
{
    if( !m_sort_clear() ) {
        log_message(LOG_ERR, "Could not clear spectra.\n");
    } else {
        logbook_message("cleared spectra", "Spectra have been cleared.");
    }
}
