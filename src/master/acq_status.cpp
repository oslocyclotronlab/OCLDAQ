
#include "acq_log.h"
#include "m_engine.h"

#include <time.h>

void acq_status()
{
    if( m_engine_status() ) {
        log_message(LOG_INFO, "Requested STATUS.\n");
    } else {
        log_message(LOG_ERR, "Problem when requesting status.\n");
    }
}
