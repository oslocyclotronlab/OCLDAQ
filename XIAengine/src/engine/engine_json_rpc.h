#ifndef ENGINE_JSON_RPC_H
#define ENGINE_JSON_RPC_H

#include "net_control.h"

#include <string>

struct EngineDaemonContext;

void engine_json_rpc_handle_line(line_channel *lc, EngineDaemonContext &ctx, const std::string &line);

#endif
