#ifndef ENGINE_STATE_H
#define ENGINE_STATE_H

/** Application lifecycle for the XIAengine daemon (Pixie + acquisition). */
enum class EngineState {
    Unconfigured,   //!< No Pixie init; crate-side config may be edited
    Initializing,   //!< Boot in progress (transient)
    Idle,           //!< Modules ready; settings allowed; not acquiring
    Run             //!< List-mode acquisition active
};

inline const char *engine_state_cstr(EngineState s)
{
    switch (s) {
    case EngineState::Unconfigured: return "unconfigured";
    case EngineState::Initializing: return "initializing";
    case EngineState::Idle: return "idle";
    case EngineState::Run: return "run";
    }
    return "unknown";
}

#endif
