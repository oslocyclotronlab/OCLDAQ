#include "engine_json_rpc.h"
#include "engine_daemon_context.h"
#include "XIAControl.h"

#include <nlohmann/json.hpp>

#include <cstring>
#include <sstream>

static void send_json(line_channel *lc, const nlohmann::json &j)
{
    std::string out = j.dump();
    out.push_back('\n');
    lc->send(out);
}

static nlohmann::json base_ok(int id)
{
    nlohmann::json r;
    r["ok"] = true;
    if (id >= 0)
        r["id"] = id;
    return r;
}

static nlohmann::json base_err(int id, const std::string &msg, EngineState st)
{
    nlohmann::json r;
    r["ok"] = false;
    if (id >= 0)
        r["id"] = id;
    r["error"] = msg;
    r["state"] = engine_state_cstr(st);
    return r;
}

static bool mutating_op(const std::string &op)
{
    return op == "set_chn_param" || op == "set_mod_param" || op == "measure_bl_cut"
        || op == "measure_baseline" || op == "copy_dsp_parameters" || op == "write_settings"
        || op == "init_boot";
}

void engine_json_rpc_handle_line(line_channel *lc, EngineDaemonContext &ctx, const std::string &line)
{
    int req_id = -1;
    try {
        nlohmann::json req = nlohmann::json::parse(line);
        if (req.contains("id") && req["id"].is_number_integer())
            req_id = req["id"].get<int>();

        if (!req.contains("op") || !req["op"].is_string()) {
            send_json(lc, base_err(req_id, "missing op", ctx.state));
            return;
        }
        const std::string op = req["op"].get<std::string>();

        if (ctx.state == EngineState::Initializing && op != "get_state" && op != "hello") {
            send_json(lc, base_err(req_id, "engine is initializing", ctx.state));
            return;
        }

        if (ctx.state == EngineState::Run && mutating_op(op)) {
            send_json(lc, base_err(req_id, "mutating operation not allowed during run", ctx.state));
            return;
        }

        if (op == "hello") {
            nlohmann::json r = base_ok(req_id);
            r["state"] = engine_state_cstr(ctx.state);
            r["nmod"] = ctx.xiacontr ? ctx.xiacontr->GetNumMod() : 0;
            r["version"] = "XIAengine 2.1";
            r["stopped"] = ctx.stopped;
            send_json(lc, r);
            return;
        }

        if (op == "get_state") {
            nlohmann::json r = base_ok(req_id);
            r["state"] = engine_state_cstr(ctx.state);
            r["nmod"] = ctx.xiacontr ? ctx.xiacontr->GetNumMod() : 0;
            r["stopped"] = ctx.stopped;
            send_json(lc, r);
            return;
        }

        if (op == "init_boot") {
            if (ctx.state != EngineState::Unconfigured) {
                send_json(lc, base_err(req_id, "init_boot only from unconfigured", ctx.state));
                return;
            }
            bool offline = req.value("offline", false);
            ctx.state = EngineState::Initializing;
            if (!ctx.xiacontr || !ctx.xiacontr->XIA_boot_all(offline)) {
                ctx.xia_iface.reset();
                if (ctx.xiacontr)
                    ctx.xiacontr->shutdownPixie();
                ctx.state = EngineState::Unconfigured;
                send_json(lc, base_err(req_id, "boot failed", ctx.state));
                return;
            }
            const size_t nmod = static_cast<size_t>(ctx.xiacontr->GetNumMod());
            if (nmod < 1) {
                ctx.xiacontr->shutdownPixie();
                ctx.state = EngineState::Unconfigured;
                send_json(lc, base_err(req_id, "no modules", ctx.state));
                return;
            }
            ctx.xia_iface = std::make_unique<XIAInterfaceAPI2>(nmod);
            ctx.state = EngineState::Idle;
            ctx.stopped = true;
            nlohmann::json r = base_ok(req_id);
            r["state"] = engine_state_cstr(ctx.state);
            r["nmod"] = static_cast<int>(nmod);
            send_json(lc, r);
            return;
        }

        if (op == "get_crate_config") {
            if (ctx.state != EngineState::Unconfigured) {
                send_json(lc, base_err(req_id, "get_crate_config only in unconfigured", ctx.state));
                return;
            }
            nlohmann::json r = base_ok(req_id);
            r["firmware_config_path"] = ctx.xiacontr->getFirmwareConfigPath();
            r["settings_file_path"] = ctx.xiacontr->getSettingsFilePath();
            send_json(lc, r);
            return;
        }

        if (op == "set_crate_config") {
            if (ctx.state != EngineState::Unconfigured) {
                send_json(lc, base_err(req_id, "set_crate_config only in unconfigured", ctx.state));
                return;
            }
            if (ctx.xiacontr->isPixieInitialized()) {
                send_json(lc, base_err(req_id, "pixie already initialized", ctx.state));
                return;
            }
            if (req.contains("firmware_config_path") && req["firmware_config_path"].is_string())
                ctx.xiacontr->setFirmwareConfigPath(req["firmware_config_path"].get<std::string>());
            if (req.contains("settings_file_path") && req["settings_file_path"].is_string())
                ctx.xiacontr->setSettingsFilePath(req["settings_file_path"].get<std::string>());
            if (!ctx.xiacontr->reloadFirmwareConfig()) {
                send_json(lc, base_err(req_id, "reloadFirmwareConfig failed", ctx.state));
                return;
            }
            send_json(lc, base_ok(req_id));
            return;
        }

        if (ctx.state != EngineState::Idle && ctx.state != EngineState::Run) {
            send_json(lc, base_err(req_id, "operation requires idle or run", ctx.state));
            return;
        }

        if (!ctx.xia_iface) {
            send_json(lc, base_err(req_id, "internal error: no interface", ctx.state));
            return;
        }
        XIAInterfaceAPI2 &api = *ctx.xia_iface;

        if (op == "get_module_info") {
            const size_t mod = req.at("module").get<size_t>();
            auto mi = api.GetModuleInfo(mod);
            nlohmann::json r = base_ok(req_id);
            r["revision"] = mi.revision;
            r["adc_bits"] = mi.adc_bits;
            r["adc_msps"] = mi.adc_msps;
            r["serial_number"] = mi.serial_number;
            send_json(lc, r);
            return;
        }

        if (op == "get_chn_limits") {
            const size_t mod = req.at("module").get<size_t>();
            const size_t ch = req.at("channel").get<size_t>();
            const std::string name = req.at("name").get<std::string>();
            auto lim = api.GetChnLimits(mod, ch, name.c_str());
            nlohmann::json r = base_ok(req_id);
            r["lo"] = lim.first;
            r["hi"] = lim.second;
            send_json(lc, r);
            return;
        }

        if (op == "get_mod_limits") {
            const size_t mod = req.at("module").get<size_t>();
            const std::string name = req.at("name").get<std::string>();
            auto lim = api.GetModLimits(mod, name.c_str());
            nlohmann::json r = base_ok(req_id);
            r["lo"] = lim.first;
            r["hi"] = lim.second;
            send_json(lc, r);
            return;
        }

        if (op == "get_chn_param") {
            const size_t mod = req.at("module").get<size_t>();
            const size_t ch = req.at("channel").get<size_t>();
            const std::string name = req.at("name").get<std::string>();
            nlohmann::json r = base_ok(req_id);
            r["value"] = api.GetChnParam(mod, ch, name.c_str());
            send_json(lc, r);
            return;
        }

        if (op == "set_chn_param") {
            const size_t mod = req.at("module").get<size_t>();
            const size_t ch = req.at("channel").get<size_t>();
            const std::string name = req.at("name").get<std::string>();
            const double val = req.at("value").get<double>();
            api.SetChnParam(mod, ch, name.c_str(), val);
            send_json(lc, base_ok(req_id));
            return;
        }

        if (op == "get_mod_param") {
            const size_t mod = req.at("module").get<size_t>();
            const std::string name = req.at("name").get<std::string>();
            nlohmann::json r = base_ok(req_id);
            r["value"] = api.GetModParam(mod, name.c_str());
            send_json(lc, r);
            return;
        }

        if (op == "set_mod_param") {
            const size_t mod = req.at("module").get<size_t>();
            const std::string name = req.at("name").get<std::string>();
            const unsigned int val = req.at("value").get<unsigned int>();
            api.SetModParam(mod, name.c_str(), val);
            send_json(lc, base_ok(req_id));
            return;
        }

        if (op == "measure_bl_cut") {
            const unsigned short mod = static_cast<unsigned short>(req.at("module").get<int>());
            const unsigned short ch = static_cast<unsigned short>(req.at("channel").get<int>());
            nlohmann::json r = base_ok(req_id);
            r["value"] = api.MeasureBLCut(mod, ch);
            send_json(lc, r);
            return;
        }

        if (op == "measure_baseline") {
            const unsigned short mod = static_cast<unsigned short>(req.at("module").get<int>());
            api.MeasureBaseline(mod);
            send_json(lc, base_ok(req_id));
            return;
        }

        if (op == "copy_dsp_parameters") {
            const unsigned short bitmap = static_cast<unsigned short>(req.at("bitmap").get<int>());
            const unsigned short sm = static_cast<unsigned short>(req.at("source_module").get<int>());
            const unsigned short sch = static_cast<unsigned short>(req.at("source_channel").get<int>());
            constexpr int kDestWords = 16 * PRESET_MAX_MODULES; // NUMBER_OF_CHANNELS * PRESET_MAX_MODULES
            std::vector<unsigned short> dest(static_cast<size_t>(kDestWords), 0);
            if (req.contains("dest_mask") && req["dest_mask"].is_array()) {
                size_t i = 0;
                for (const auto &el : req["dest_mask"]) {
                    if (i >= dest.size())
                        break;
                    dest[i++] = static_cast<unsigned short>(el.get<int>());
                }
            }
            int rv = api.CopyDSPParameters(bitmap, sm, sch, dest.data());
            nlohmann::json r = base_ok(req_id);
            r["retval"] = rv;
            r["dest_mask"] = dest;
            send_json(lc, r);
            return;
        }

        if (op == "write_settings") {
            const std::string path = req.at("path").get<std::string>();
            nlohmann::json r = base_ok(req_id);
            r["saved"] = api.WriteSettings(path.c_str());
            send_json(lc, r);
            return;
        }

        send_json(lc, base_err(req_id, "unknown op: " + op, ctx.state));
    } catch (const std::exception &ex) {
        send_json(lc, base_err(req_id, ex.what(), ctx.state));
    }
}
