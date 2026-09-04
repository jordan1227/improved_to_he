#!/usr/bin/env python3
"""Static roster check for the scripted weapon-control formula."""

from __future__ import annotations

import math
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROFILE = ROOT / "gamedata/scripts/kotovod/sivol_fuzz_recoil_profiles.script"
WEAPONS = ROOT / "gamedata/config/weapons"


def lua_block(source: str, name: str, next_marker: str) -> str:
    match = re.search(
        rf"local {re.escape(name)} = \{{(.*?)\n\}}\n\n{re.escape(next_marker)}",
        source,
        re.S,
    )
    if not match:
        raise RuntimeError(f"Lua table not found: {name}")
    return match.group(1)


def numbers(block: str) -> dict[str, float]:
    return {
        key: float(value)
        for key, value in re.findall(
            r"([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(-?\d+(?:\.\d+)?)", block
        )
    }


def named_tables(block: str) -> dict[str, dict[str, float]]:
    result: dict[str, dict[str, float]] = {}
    for name, body in re.findall(
        r"^\s{4}([A-Za-z_][A-Za-z0-9_]*)\s*=\s*\{(.*?)^\s{4}\},",
        block,
        re.M | re.S,
    ):
        result[name] = numbers(body)
    return result


def read_ltx() -> dict[str, tuple[list[str], dict[str, str]]]:
    sections: dict[str, tuple[list[str], dict[str, str]]] = {}
    for path in WEAPONS.glob("*.ltx"):
        current: tuple[list[str], dict[str, str]] | None = None
        for raw in path.read_text(encoding="cp1251").splitlines():
            line = raw.split(";", 1)[0].strip()
            if not line:
                continue
            header = re.match(r"^\[([^]]+)]\s*(?::\s*(.*))?$", line)
            if header:
                name = header.group(1).strip()
                parents = [part.strip() for part in (header.group(2) or "").split(",") if part.strip()]
                current = sections.setdefault(name, (parents, {}))
                continue
            if current and "=" in line:
                key, value = line.split("=", 1)
                current[1][key.strip()] = value.strip()
    return sections


def effective(
    sections: dict[str, tuple[list[str], dict[str, str]]],
    section: str,
    key: str,
    visited: set[str] | None = None,
) -> str | None:
    visited = visited or set()
    if section in visited or section not in sections:
        return None
    visited.add(section)
    parents, values = sections[section]
    if key in values:
        return values[key]
    for parent in parents:
        value = effective(sections, parent, key, visited)
        if value is not None:
            return value
    return None


def clamp(value: float, low: float, high: float) -> float:
    return max(low, min(high, value))


def main() -> None:
    source = PROFILE.read_text(encoding="utf-8")
    global_recoil = numbers(lua_block(source, "GLOBAL_RECOIL", "local DEFAULT_PERSONALITY"))
    default = numbers(lua_block(source, "DEFAULT_PERSONALITY", "local WEAPON_PERSONALITY"))
    classes = named_tables(lua_block(source, "CLASS_PROFILES", "-- Every base item"))
    section_classes = dict(
        re.findall(
            r"(wpn_[A-Za-z0-9_]+)\s*=\s*\"([A-Za-z_]+)\"",
            lua_block(source, "SECTION_CLASS", "local EXCLUDED_SECTIONS"),
        )
    )
    personalities: dict[str, dict[str, float]] = {}
    for ids, body in re.findall(
        r"register_personality\(\{(.*?)\},\s*\{(.*?)\}\)", source, re.S
    ):
        profile = default | numbers(body)
        for section in re.findall(r'\"(wpn_[A-Za-z0-9_]+)\"', ids):
            personalities[section] = profile

    sections = read_ltx()
    scores: dict[str, tuple[int, int | None]] = {}
    errors: list[str] = []
    for section, class_name in sorted(section_classes.items()):
        class_profile = classes[class_name]
        personality = personalities.get(section, default)
        raw: dict[str, float] = {}
        for key in ("cam_dispersion", "cam_dispersion_inc", "cam_relax_speed", "cam_step_angle_horz", "ammo_mag_size", "rpm"):
            value = effective(sections, section, key)
            try:
                raw[key] = float(value) if value is not None else math.nan
            except ValueError:
                raw[key] = math.nan
        missing = [key for key, value in raw.items() if not math.isfinite(value)]
        if missing:
            errors.append(f"{section}: unavailable {','.join(missing)}")
            continue

        magazine_factor = clamp(1 + (30 - raw["ammo_mag_size"]) / 30 * 0.10, 0.88, 1.12)
        fire_rate_factor = clamp(raw["rpm"] / 600, 0.78, 1.24)
        recovery_rate = clamp(
            raw["cam_relax_speed"] * class_profile["recovery_scale"]
            * personality.get("recovery", 1) * global_recoil.get("recovery", 1),
            0.55,
            16,
        )
        automatic = "-1" in (effective(sections, section, "fire_modes") or "")
        heat = vertical_total = horizontal_total = first_vertical = 0.0
        for shot in range(1, 7):
            heat_gain = personality.get("heat_rate", 0.055)
            if shot <= personality.get("heat_grace", 6):
                heat_gain *= 0.20
            heat = clamp(heat + heat_gain, 0, 1)
            burst_index = clamp(
                (shot - 1) * 0.18,
                0,
                min(1.80, max(0, class_profile["max_burst"] - 1)),
            )
            stage = personality.get("first", 1) if shot == 1 else (
                personality.get("second", 1) if shot == 2 else personality.get("sustained", 1)
            )
            heat_k = 1 + heat * (personality.get("heat_max", 1.55) - 1)
            vertical = raw["cam_dispersion"] * class_profile["dispersion_scale"]
            vertical += raw["cam_dispersion_inc"] * class_profile["dispersion_increment_scale"] * burst_index
            vertical *= magazine_factor
            vertical *= 1 + class_profile["burst_scale"] * burst_index * fire_rate_factor
            vertical *= personality.get("pitch", 1) * stage * (1 + (heat_k - 1) * 0.65)
            if shot == 1:
                vertical = max(vertical, personality.get("first_pitch_floor", 0))
            balance = global_recoil.get("vertical_multiplier", 1)
            if shot == 1 and automatic:
                balance *= global_recoil.get("automatic_first_shot_multiplier", 1)
            vertical = min(
                vertical * balance * class_profile["ads_multiplier"]
                * global_recoil.get("overall", 1) * global_recoil.get("camera_pitch", 1),
                class_profile["max_pitch_deg"],
            )
            horizontal = max(
                raw["cam_step_angle_horz"] * class_profile["horizontal_step_scale"],
                raw["cam_dispersion"] * class_profile["horizontal_dispersion_scale"],
            )
            horizontal *= 1 + class_profile["burst_scale"] * burst_index * 0.35
            horizontal *= personality.get("yaw", 1) * (
                1 + (heat_k - 1) * (1.35 + 0.45 * personality.get("wander", 1))
            )
            horizontal = min(
                horizontal * class_profile["ads_multiplier"]
                * global_recoil.get("overall", 1) * global_recoil.get("camera_yaw", 1),
                class_profile["max_yaw_deg"],
            )
            if shot == 1:
                first_vertical = vertical
            if shot >= 3:
                vertical_total += vertical
            horizontal_total += horizontal

        def curve(value: float, midpoint: float, exponent: float) -> float:
            return 100 / (1 + (max(0, value) / midpoint) ** exponent)

        first = curve(first_vertical, 1.80, 1.20)
        sustained = curve(vertical_total / 4, 1.60, 1.20)
        horizontal = curve(horizontal_total / 6, 0.45, 1.25)
        recovery = 100 * recovery_rate / (recovery_rate + 2.50)
        single = round(clamp(first * 0.55 + horizontal * 0.15 + recovery * 0.30, 1, 95))
        auto = round(clamp(first * 0.20 + sustained * 0.50 + horizontal * 0.15 + recovery * 0.15, 1, 95)) if automatic else None
        scores[section] = (single, auto)

    checks = {
        "PM and APB primary control differ": scores["wpn_pm"][0] != (scores["wpn_aps"][1] or scores["wpn_aps"][0]),
        "full MP-153 beats short MP-153": scores["wpn_winchester_m1"][0] > scores["wpn_mp153_m1"][0],
        "full MP-153 differs from Benelli M4": scores["wpn_winchester_m1"][0] != scores["wpn_spas12_m1"][0],
    }
    for label, passed in checks.items():
        if not passed:
            errors.append(f"failed relation: {label}")

    print(f"Handling validation: scored={len(scores)} errors={len(errors)}")
    for section in ("wpn_pm", "wpn_aps", "wpn_winchester_m1", "wpn_mp153_m1", "wpn_mp153", "wpn_spas12_m1", "wpn_rpk74"):
        single, auto = scores[section]
        print(f"{section}: single={single}" + (f" auto={auto}" if auto is not None else ""))
    for error in errors:
        print(f"ERROR: {error}")
    if errors:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
