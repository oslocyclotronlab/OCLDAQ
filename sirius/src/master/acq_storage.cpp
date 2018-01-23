
#include <sstream>
#include <time.h>

#include "acq_log.h"
#include "m_engine.h"
#include "m_sort.h"

#include "run_command.h"
extern command_list* commands;

void acq_storage( bool file )
{
    if( !file ) {
        if( !m_engine_output_none() ) {
            log_message(LOG_ERR, "while trying to set engine output to none.\n");
        } else {
            logbook_message("output: none", "Data is not written to disk.");
        }
    } else {
        time_t now = time(0);

        char buf[1024];
        strftime(buf, sizeof(buf), "sirius-%Y%m%d-%H%M%S.data", localtime(&now));

        if( !m_engine_output_filename(buf) ) {
            log_message(LOG_ERR, "while trying to set engine output to '%s'.\n", buf);
        } else {
            int sb, se;
            float sal;
            m_sort_get_buffers(sb, se, sal);
            
            std::ostringstream txt;
            txt << "Data is written to file '" << buf << "'.\n"
                << "Please edit this message and include:\n"
                << "trigger rate: \n"
                << "beam intensity: \n"
                << "event length: (" << sal << ")\n";
            logbook_message("output: file", txt.str());
        }
    }
}
