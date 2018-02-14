
#include "acq_log.h"
#include "m_engine.h"

#include <time.h>

void acq_reload()
{
	if ( m_engine_reload() ){
		log_message(LOG_INFO, "Requested RELOAD.\n");
	} else {
		log_message(LOG_ERR, "Problem when requesting reload.\n");
	}

}