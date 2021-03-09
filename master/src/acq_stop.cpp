
#include "acq_log.h"
#include "m_engine.h"

#include "run_command.h"
extern command_list* commands;

void acq_stop()
{
    if( !m_engine_stop() ) {
        log_message(LOG_ERR, "while trying to stop engine.\n");
    } else {
        //logbook_message("stopped acquisition", "The acquisition has been stopped.");
    }
}
