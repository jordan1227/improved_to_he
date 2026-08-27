# Dialogue modding in this STALKER/OGSR checkout

## The main rule

A dialogue is not contained in one `dialogs_*.xml` file. A working dialogue is
a chain of separately loaded pieces:

```text
system.ltx registries
    -> gameplay XML roots and includes
    -> <dialog> and <phrase_list>
    -> character start/actor binding or a runtime meet scheme
    -> declared infoportions and their state transitions
    -> loaded localization <string> keys
    -> script preconditions/actions and runtime overrides
```

Before editing, trace all of those layers. Static XML can show that the pieces
exist, but only an in-game test proves that the intended NPC offers the
dialogue, that conditions select the intended branch, and that script side
effects work.

## Where each piece lives

| Layer | Current source of truth | What to check |
| --- | --- | --- |
| Dialogue loading | `gamedata/config/system.ltx`, `[dialogs]` | The file basename is listed, or the file is included by a listed root. |
| Dialogue trees | `gamedata/config/gameplay/dialogs*.xml` and included fragments | The `<dialog id>` exists, phrase links resolve, and its conditions/actions are intentional. |
| NPC availability | `character_desc_*.xml`, `character_dialogs*.xml`, and sometimes `gamedata/config/misc/gulag_*.ltx` | `start_dialog`, `actor_dialog`, or a runtime `meet_dialog` path exposes the dialogue. |
| Persistent state | `gamedata/config/system.ltx`, `[info_portions]`; `gamedata/config/gameplay/info_*.xml` | Every new info ID used by a condition or transition is declared in a loaded info file. |
| Text | `gamedata/config/system.ltx`, `[string_table]`; `gamedata/config/text/rus` and `eng` | Every `<text>` value resolves to an exact loaded `<string id>` in each language that the target build supports. |
| Dynamic dialogue | `gamedata/scripts/dialogs/dialog_manager.script`, `gamedata/config/misc/dialog_manager.ltx`, and manager text tables | `dm_*` shells are populated by script; their static XML is not the whole dialogue. |

The current `[dialogs]` registry loads the main roots, including
`dialogs_trading`. That root includes many fragment files such as
`dialogs_story.xml`, `dialogs_other.xml`, `dialogs_cycle_task.xml`, and the
`sak_*` dialogue fragments. Do not assume that every physical file whose name
contains `dialogs` is independently loaded.

The same principle applies to text. `sak_strings.xml` is a loaded text root
that includes fragments such as `story_dialogs.xml` and `doc_dialogs.xml`; an
unlisted fragment is not loaded merely because it exists on disk.

## 1. Trace an existing dialogue before creating one

Use an existing nearby dialogue as the template and trace it in this order:

1. Find its exact `<dialog id="...">` in `gamedata/config/gameplay`.
2. Follow the containing file back to a root listed in `system.ltx` and through
   any `#include` chain.
3. Find every character reference to that ID in
   `character_desc_*.xml`, `character_dialogs*.xml`, `gulag_*.ltx`, and relevant
   scripts.
4. Read every `<has_info>`, `<dont_has_info>`, `<give_info>`,
   `<disable_info>`, `<precondition>`, and `<action>` in the dialogue.
5. Resolve every nonempty `<text>` key in the loaded language tables.
6. Check for `meet_dialog`, wounded-dialog, dialog-manager, or other runtime
   code that can replace or add the static dialogue.

This avoids debugging a text key when the real problem is that the dialogue
was never loaded or never attached to the NPC.

## 2. Static dialogue tree structure

The normal shape is:

```xml
<dialog id="example_dialog">
    <precondition>dialogs.is_not_wounded</precondition>
    <has_info>example_quest_start</has_info>
    <dont_has_info>example_quest_done</dont_has_info>
    <phrase_list>
        <phrase id="0">
            <text>example_dialog_0</text>
            <next>1</next>
            <next>10</next>
        </phrase>
        <phrase id="1">
            <text>example_dialog_1</text>
            <give_info>example_quest_start</give_info>
            <next>2</next>
        </phrase>
        <phrase id="10">
            <text>example_dialog_10</text>
            <action>dialogs.break_dialog</action>
        </phrase>
    </phrase_list>
</dialog>
```

- `dialog id` is the global name referenced by profiles, schemes, and scripts.
  Keep it unique.
- A dialog-level `precondition`, `has_info`, or `dont_has_info` gates the whole
  dialogue. Phrase-level versions gate a specific branch. Treat multiple
  neighboring conditions as a combined gate and preserve the local pattern.
- `phrase id` values are local to that dialogue. They do not need to be
  sequential, but every `<next>` target must exist in the same phrase list.
- One `<next>` continues linearly; several `<next>` elements make choices.
  A phrase without `<next>` is commonly terminal. Some branches explicitly
  call `dialogs.break_dialog` as well.
- An empty `<text />` or `<text></text>` is often an intentional routing node
  used to select a branch by info conditions. Do not “fix” it by inventing a
  localization key without checking the surrounding tree.
- `<text>` contains a string-table key, not the visible sentence. The visible
  text belongs in a loaded localization file.
- `<precondition>` and `<action>` contain an existing script function, with
  optional arguments in the project’s established call syntax. Verify the
  function in the scripts before using it; do not invent a callback name.
- `<give_info>` advances persistent game state. `<disable_info>` is used on
  phrase branches to remove or invalidate another info state. Multiple state
  transitions can occur on one phrase.
- `<priority>` is used by some dialog shells to influence selection order among
  competing dialogues. Preserve nearby values and verify the winner in game;
  the XML alone is not enough to infer every selection detail.

For a quest branch, the whole-dialog gate and the final phrase often look like
this in the existing files:

```xml
<dialog id="bar_bar_hunter_quest_finish">
    <precondition>sak.have_items_count( "wpn_toz34_m1", 1 )</precondition>
    <has_info>bar_bar_hunter_quest_start</has_info>
    <dont_has_info>bar_bar_hunter_quest_finish</dont_has_info>
    <phrase_list>
        <!-- intermediate phrases omitted -->
        <phrase id="11">
            <text>bar_bar_hunter_quest_finish_11</text>
            <give_info>bar_bar_hunter_quest_finish</give_info>
            <!-- reward actions follow the local project pattern -->
        </phrase>
    </phrase_list>
</dialog>
```

The item check, info state, text, and reward action are separate contracts;
changing one does not automatically provide the others.

## 3. Infoportions are a required second half of the dialogue

The `[info_portions]` list in `system.ltx` is the loader for the state
definitions. In this checkout it includes `info_portions_nlc30.xml`.

When adding NLC-specific dialogue state, add the new IDs to
`gamedata/config/gameplay/info_portions_nlc30.xml`, not only to a dialogue
tree. This applies to every new ID used by:

- `<has_info>` and `<dont_has_info>` conditions;
- `<give_info>` and `<disable_info>` in phrase nodes;
- script calls that give or disable an infoportion;
- infoportion metadata that disables another state, opens/closes an article,
  starts a task, or calls a script action.

Use another loaded `info_*.xml` only when the state genuinely belongs to that
existing subsystem. Search for an existing definition first so that one ID is
not declared twice with conflicting metadata.

A minimal state can be a bare declaration:

```xml
<info_portion id="example_quest_start"></info_portion>
```

The current files also use metadata on an infoportion, for example:

```xml
<info_portion id="bar_dolg_leader_find_case_failed">
    <disable_info>bar_dolg_leader_find_case_start</disable_info>
    <article_disable>tex_bar_dolg_leader_find_case</article_disable>
</info_portion>
```

Existing metadata forms include:

- `<disable>` and `<disable_info>` for related info-state cleanup. They are
  separate forms in the current data; copy the convention of a comparable
  state machine instead of treating them as interchangeable.
- `<article>` and `<article_disable>` for PDA/encyclopedia article lifecycle.
  Trace the article ID through `storyline_info_*.xml` or the relevant
  encyclopedia file, then resolve its name and body text too.
- `<task>` for a task association.
- `<action>` for a script callback fired as part of info acquisition.

`info_portions_nlc30.xml` intentionally contains some bare declarations. Its
header documents that some original NLC article/action references pointed at
content absent from this OGSR base. Do not restore those fields automatically:
verify the target article, text, and script function first.

The practical state-machine pattern is usually:

```xml
<info_portion id="example_quest_start"></info_portion>
<info_portion id="example_quest_done">
    <disable_info>example_quest_start</disable_info>
</info_portion>
```

Then the dialogue uses `give_info` to move from start to done. Whether an
additional failure state, article, task, or cleanup action is needed is a
design decision that must follow the neighboring quest’s pattern.

## 4. Text and localization

For a gameplay node such as:

```xml
<text>example_dialog_0</text>
```

the loaded text data must contain the exact key:

```xml
<string id="example_dialog_0">
    <text>Visible sentence goes here.</text>
</string>
```

Check all of the following:

- the table is listed in `[string_table]` in `system.ltx`, or is included by a
  listed root;
- the key is unique and spelled exactly the same as the gameplay reference;
- the key exists in every supported language table;
- XML-sensitive characters and intentional `\n` sequences are encoded using
  the project’s existing style;
- the key naming follows the nearby dialogue, commonly
  `<dialog_id>_<phrase_id>`.

This checkout has Russian and English language directories, but the current
NLC-specific `stable_dialogs_nlc30.xml` is present under `rus` without a
matching `eng` file. Check the actual target language coverage before claiming
that a new dialogue is localized in both languages; do not assume a missing
language table will safely fall back.

Adding a string key does not attach a dialogue to an NPC, create an infoportion,
or execute an action. It only supplies presentation text.

## 5. Make the dialogue available to the right NPC

Static character profiles commonly contain:

```xml
<start_dialog>hello_dialog</start_dialog>
<actor_dialog>bar_bar_hunter_quest_start</actor_dialog>
```

- `start_dialog` is the NPC’s default conversation entry for the relevant
  profile/meet path.
- `actor_dialog` adds a dialogue to the actor-initiated choices for that NPC.

These entries are profile-local. `character_dialogs.xml` and its numbered
variants are small fragments included inside character profiles; they are not
global dialogue registries. The profile files themselves follow the separate
`[profiles]` registry in `system.ltx` (currently `npc_profile` and
`npc_profile_nlc30_spawn`). A valid `<dialog>` with no matching character or
scheme binding will not necessarily appear in conversation.

The default profile can also be changed at runtime. Inspect `meet_dialog` in
the relevant `gamedata/config/misc/gulag_*.ltx` and the handling in
`gamedata/scripts/xr/xr_meet.script`. Wounded NPC logic in
`gamedata/scripts/xr/xr_wounded.script` can provide another dialog path.
Runtime condlists and script code can replace the profile default, so changing
only `start_dialog` may have no visible effect.

When a dialogue is missing from the menu, check in this order:

1. Is its XML root or include actually loaded?
2. Is the ID bound by `start_dialog`, `actor_dialog`, or a runtime scheme?
3. Do dialog-level and phrase-level info/precondition gates pass?
4. Are the referenced infoportions declared and in the expected state?
5. Did `meet_dialog`, wounded logic, or another script override the binding?
6. Does the runtime log or an in-game test show a script/text failure?

## 6. Dynamic `dm_*` dialogues

Several entries in `dialogs.xml` are shells with an `init_func`, for example a
`dialog_manager.init_*_dialog`, rather than a static `<phrase_list>`. The
actual phrases and conditions are assembled by
`gamedata/scripts/dialogs/dialog_manager.script` from
`gamedata/config/misc/dialog_manager.ltx` and the manager text tables.

For a `dm_*` dialogue:

- inspect the shell’s `init_func`;
- read the corresponding initializer in `dialog_manager.script`;
- trace the category/condition data in `dialog_manager.ltx`;
- add or change the manager’s localization keys in the loaded manager text
  table;
- do not add a static phrase to the shell and assume the manager will use it.

The manager can add preconditions, actions, and info transitions when it builds
the native dialogue object. Those effects are script-driven and require an
in-game test.

## 7. Safe editing and validation checklist

For a new or changed quest dialogue:

1. Prefer an existing loaded root or include fragment. Add a new top-level
   dialogue file to `[dialogs]` only when there is a reason not to extend an
   existing aggregation root.
2. Give the dialog a unique ID and keep phrase IDs unique within it. Check all
   `<next>` targets.
3. Declare every new state ID in a loaded info file; use
   `info_portions_nlc30.xml` for new NLC-specific IDs.
4. Add every visible phrase key to the correct loaded language table and check
   the other language directory separately.
5. Bind the dialogue through the appropriate character profile or runtime
   scheme.
6. Resolve every precondition/action to an existing function and verify its
   arguments against a nearby call.
7. Trace article/task/action metadata before adding it to an infoportion.
8. Check for `meet_dialog`, wounded, dialog-manager, and other runtime
   overrides.
9. Preserve runtime XML conventions: Windows-1251 without a BOM where that is
   the existing file format, original line endings, and literal Cyrillic. The
   Markdown documentation itself is a normal UTF-8 document and must not be
   copied to the runnable installation.
10. Run structural/reference checks and `git diff --check`. Remember that
    include fragments may not be valid standalone XML; validate the appropriate
    expanded/root context.
11. If testing is requested, deploy only the changed runtime files to
    `C:\Games\NLC_OGSR_HE`, compare source/target hashes, and report manual
    game results separately from static checks. Documentation-only work is not
    deployed.

## Common traps

- Creating `dialogs_new.xml` without listing it in `[dialogs]` or including it
  from a loaded root.
- Adding a `<dialog>` but forgetting the NPC’s `start_dialog` or
  `actor_dialog` binding.
- Using a new info ID in `<has_info>`/`<give_info>` without declaring it in a
  loaded `info_*.xml`, especially `info_portions_nlc30.xml` for NLC state.
- Adding text to an unlisted fragment, or checking only Russian while the
  target build uses another language table.
- Treating an empty-text routing phrase as a broken localization entry.
- Calling a script function that exists in another mod branch but not in this
  checkout.
- Editing a static profile while `meet_dialog` or wounded/runtime logic wins
  later.
- Restoring article/action metadata from original NLC data without verifying
  that the referenced OGSR article, string keys, and script callback exist.
- Treating an XML parse, reference audit, hash comparison, or diff as proof of
  in-game dialogue behavior.
