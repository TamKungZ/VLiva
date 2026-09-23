# VLiva OBS source plugin

This standalone OBS Studio source plugin reads RGBA frames from VLiva's shared
memory protocol and exposes them as an OBS source named **VLiva Capture**.

The plugin is intentionally isolated from the proprietary VLiva application:
it links to OBS/libobs and communicates with VLiva only through a documented
shared-memory layout. All code required to build the plugin is contained in
this directory.

## License

Copyright (c) 2026 TamKungZ_. This directory is licensed under
GPL-2.0-or-later because it links with OBS Studio/libobs. See `LICENSE`.
This directory-specific license overrides the repository-level proprietary
license only for files contained under `plugins/obs`.

## Build

Install the OBS Studio development package, then run:

```bash
cmake -S plugins/obs -B build/obs-plugin \
  -DVLIVA_OBS_PLUGIN_REQUIRE_OBS=ON
cmake --build build/obs-plugin --parallel
```

The module is written to `build/obs-plugin/vliva_obs_source.so`. Install it
with:

```bash
cmake --install build/obs-plugin --prefix "$HOME/.local"
```

The default shared-memory session is `/vliva_obs_main`. It can be changed in
the source properties inside OBS.

## Protocol compatibility

The protocol is versioned by `kSharedFrameVersion` in
`include/vliva/capture/shared_frame_protocol.hpp`. Changes to the private VLiva
publisher must preserve this layout or update this public mirror in the same
release.
