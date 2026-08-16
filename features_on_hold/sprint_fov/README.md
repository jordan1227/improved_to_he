# Sprint FOV — on hold

This experimental feature is intentionally kept outside `gamedata` so the game cannot load it.

The stock Lua camera API does not provide a reliable per-frame FOV override at the point required by the camera pipeline. Continuing this feature requires an engine or reverse-hook addition in `CActorCameraManager`, exposed to Lua with explicit cleanup for ADS, scripted cameras, actor destruction, and save/load transitions.

`sivol_sprint_fov.script` is retained here as a reference implementation for speed sampling, smoothing, state suppression, and base-FOV tracking. It must not be registered in `ogse_signals_addons_list.script` unless the required engine binding is implemented and the script is updated to use it.
