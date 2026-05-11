#ifndef ENGINE_DAEMON_CONTEXT_H
#define ENGINE_DAEMON_CONTEXT_H

#include "engine_state.h"
#include "xiainterface2.h"

#include <memory>

class XIAControl;

/** Shared daemon state (single engine thread). */
struct EngineDaemonContext {
    EngineState state = EngineState::Unconfigured;
    XIAControl *xiacontr = nullptr;
    std::unique_ptr<XIAInterfaceAPI2> xia_iface;
    /** Acquisition "stopped" flag (meaningful in Idle/Run). */
    bool stopped = true;
};

#endif
