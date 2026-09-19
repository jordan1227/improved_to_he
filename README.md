# Hard Edition to Improved 3.9.1

Quests, mechanics and features of **NLC Hard Edition** (NLC 3.0) ported into
**NLC 3.9.1 OGSR** — an OpenXRay-based build of S.T.A.L.K.E.R.: Shadow of Chernobyl.

## Status

**Development is complete.** This repository holds the source tree of the finished mod.

## Installation

> [!NOTE]
> _To be filled in by the author._

A new game is required — the mod ships its own `all.spawn`, so saves from the base build will not load.

## Repository layout

| Path | Contents |
|------|----------|
| `gamedata/` | Main mod content: scripts, configs, textures, sounds, localization |
| `all.spawn.improved_3.9.1_ogsr/` | Compiled `all.spawn` and its unpacked LTX sources |
| `all.spawn_NLC.HE/` | Reference `all.spawn` from NLC Hard Edition |
| `fl_hook/` | Engine hook: `flhook.cpp` source and the built `dinput8.dll` |
| `bin_x64/` | `he_gui_debugbin.dll` |
| `Опционально/` | Optional packs, see below |
| `fsgame.ltx`, `he_gui.dll` | Game configuration and UI library |

## Optional packs

Each pack is applied by merging its `gamedata` folder over the main one.

- **`оружие/`** — two weapon packs: `WEAPONS_NLC30` and `WEAPONS_NLC_ALT`
- **`погода/`** — five weather sets: `CGIM_Origin`, `N7_Original`, `SA_NLC_Style`, `SWTC_Call_Of_Pripyat`, `VIVID`
- **`болтяры/`** — three bolt models: `2.0_dsh_anim_fix`, `3.0`, `7.5.3`
- **`arrival_anomaly_vfx/`** — ARRIVAL anomaly visuals and sounds, presentation only

## Engine hook

`fl_hook/flhook.cpp` is a proxy DLL built as `dinput8.dll`. It adds console commands and
Lua bindings used by the mod. The game executable itself is not patched.

Build: `g++ -O2 -shared -static` (MinGW/MSYS2).

## Feedback

Report issues and suggestions through Issues.

## Credits

jordan1227 · AiramProvoker · KotovodKoteikovich

---

# Hard Edition на Improved 3.9.1

Квесты, механики и возможности **NLC Hard Edition** (NLC 3.0), перенесённые в
**NLC 3.9.1 OGSR** — сборку S.T.A.L.K.E.R.: Тень Чернобыля на движке OpenXRay.

## Статус

**Разработка завершена.** В репозитории лежит исходное дерево готовой сборки.

## Установка

> [!NOTE]
> _Заполняется автором._

Нужна новая игра — сборка идёт со своим `all.spawn`, старые сейвы не загрузятся.

## Состав репозитория

| Путь | Содержимое |
|------|------------|
| `gamedata/` | Основное содержимое: скрипты, конфиги, текстуры, звуки, локализация |
| `all.spawn.improved_3.9.1_ogsr/` | Собранный `all.spawn` и его распакованные LTX-исходники |
| `all.spawn_NLC.HE/` | Исходный `all.spawn` из NLC Hard Edition, для сверки |
| `fl_hook/` | Движковый хук: исходник `flhook.cpp` и собранный `dinput8.dll` |
| `bin_x64/` | `he_gui_debugbin.dll` |
| `Опционально/` | Опциональные паки, см. ниже |
| `fsgame.ltx`, `he_gui.dll` | Конфигурация игры и библиотека интерфейса |

## Опциональные паки

Каждый пак ставится наложением его папки `gamedata` поверх основной.

- **`оружие/`** — два оружейных пака: `WEAPONS_NLC30` и `WEAPONS_NLC_ALT`
- **`погода/`** — пять погодных наборов: `CGIM_Origin`, `N7_Original`, `SA_NLC_Style`, `SWTC_Call_Of_Pripyat`, `VIVID`
- **`болтяры/`** — три модели болта: `2.0_dsh_anim_fix`, `3.0`, `7.5.3`
- **`arrival_anomaly_vfx/`** — эффекты и звуки аномалий из ARRIVAL, только внешняя часть

## Движковый хук

`fl_hook/flhook.cpp` — прокси-DLL, собирается как `dinput8.dll`. Добавляет консольные
команды и Lua-функции, которыми пользуется сборка. Сам исполняемый файл игры не патчится.

Сборка: `g++ -O2 -shared -static` (MinGW/MSYS2).

## Обратная связь

Ошибки и предложения можно отправлять через Issues.

## Авторы

jordan1227 · AiramProvoker · KotovodKoteikovich
