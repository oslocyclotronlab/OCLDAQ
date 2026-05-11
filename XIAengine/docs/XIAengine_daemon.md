# XIAengine daemon (headless) and remote GUI

## Processes

- **XIAengine** — Headless daemon: Pixie16, shared memory, and TCP control. No Qt GUI is embedded in this binary.
- **xiaconfigurator** — Separate Qt application that connects to the daemon over TCP (JSON-RPC on port **32010**).

## TCP ports

| Port  | Protocol        | Purpose |
|-------|-----------------|---------|
| 32009 | Sirius line text | Acquisition control (`start`, `stop`, `status`, `quit`, output file/dir, …). Same behaviour as before, with **state-aware** `start` (requires **idle** after boot). |
| 32010 | JSON one line per message | Settings and crate configuration. Each message is a single UTF-8 JSON object terminated by `\n`. |

## Engine states

Responses from `status` on 32009 include a line `206 engine_state <name>`. JSON `hello` / `get_state` include `"state"`.

1. **unconfigured** — Pixie not initialized. Use JSON `init_boot` (or start the daemon with `--auto-boot`) to move to **idle**.
2. **initializing** — Boot in progress; most RPC calls are rejected.
3. **idle** — Modules ready; settings RPC allowed; acquisition not running.
4. **run** — List-mode run active. **Mutating** settings RPC (`set_*`, `measure_*`, `copy_dsp_parameters`, `write_settings`) is rejected until `stop` on 32009.

## Starting the daemon

```text
XIAengine [--auto-boot] [--offline] [PXI slot numbers...]
```

- With no slot arguments, slot mapping is read from `pxisys.ini` (same as before).
- **`--auto-boot`**: run the legacy boot sequence immediately (optional convenience for a crate PC).
- Without **`--auto-boot`**, the daemon stays **unconfigured** until a client sends **`{"op":"init_boot"}`** on port 32010 (and optional `"offline": true`).

## JSON-RPC (32010)

### Handshake and boot

- **`{"op":"hello"}`** — Returns `ok`, `state`, `nmod`, `version`, `stopped`.
- **`{"op":"get_state"}`** — Same useful fields.
- **`{"op":"init_boot", "offline": false}`** — Only from **unconfigured**. On success: `nmod`, `state` becomes idle on the next query.

### Unconfigured crate paths

- **`{"op":"get_crate_config"}`** — Returns `firmware_config_path`, `settings_file_path` (paths on the **daemon host**).
- **`{"op":"set_crate_config", "firmware_config_path": "...", "settings_file_path": "..."}`** — Optional keys; reloads the firmware list from disk (still **unconfigured**).

### Settings (idle or run; mutating ops rejected in **run**)

Operations mirror the in-process `XIAInterface`: `get_module_info`, `get_chn_limits`, `get_mod_limits`, `get_chn_param`, `set_chn_param`, `get_mod_param`, `set_mod_param`, `measure_bl_cut`, `measure_baseline`, `copy_dsp_parameters` (with `dest_mask` array of length **384** = 16×24), `write_settings` with `"path"` on the daemon filesystem.

Optional integer **`id`** in each request is echoed in the response.

## Remote GUI

The installed binary name is **`xiaconfigurator`** (CMake target `xia_configurator_client` uses `OUTPUT_NAME` to avoid a Qt autogen quirk).

```text
xiaconfigurator <host> [port] [--offline]
```

Default `port` is **32010**. If the daemon reports **unconfigured**, the client sends **`init_boot`** automatically (honouring **`--offline`**).

## Security

There is no authentication in this revision; bind to a trusted network or use SSH port forwarding.
