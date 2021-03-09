
#include "acq_log.h"
#include "m_engine.h"

void acq_start()
{
    if( !m_engine_start() ) {
        log_message(LOG_ERR, "while trying to start engine.\n");
    } else {
        //logbook_message("started acquisition", "The acquisition has been started.");
    }
}
