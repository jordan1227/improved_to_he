# Gameplay menu option workflow

Use this procedure for a custom Lua-backed gameplay checkbox. It records the
project-specific pattern supplied by the maintainer on 2026-08-16.

1. Declare the persistent field and its reset default in
   `gamedata/scripts/kotovod/game_options.script`.
2. Add English and Russian label/description strings in the relevant
   `gamedata/config/text/*/ui_st_mm.xml` files.
3. Add a checkbox in `gamedata/config/ui/ui_mm_opt.xml` under `tab_gameplay`.
   Keep an `options_item` element for schema consistency.
4. Register the control in `ui_mm_opt_gameplay.script` with the sixth table
   field set to `true`; that stores the live control on the menu handler.
5. In `ui_mm_opt_main.script`, load it with `SetCheck(game_options.Field)` and
   save it with `game_options.Field = self.control:GetCheck()`.
6. Read `game_options.Field` at the gameplay feature's decision point.

The engine-cvar five-field pattern must only be used for an existing console
variable. Custom settings require the script-backed pattern above so they load,
save, reset, and cancel correctly.
