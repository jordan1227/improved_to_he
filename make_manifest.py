#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Generate update/manifest.txt for the NLC launcher.

The manifest lists every file the launcher keeps in sync, hashed over the
*blob* bytes git stores -- i.e. exactly what raw.githubusercontent.com serves.
That matters because the repo is checked out with core.autocrlf=true, so the
working tree and the served bytes are not the same for text files.

Usage:
    python make_manifest.py --apply          write to_git/manifest.txt
    python make_manifest.py --verify         check the committed manifest
    python make_manifest.py --apply --rev HEAD --repo-dir D:\\impr\\impr\\to_git

    --apply also refreshes the main-menu patch stamp in both language files.
    Use --no-menu-version to leave those files unchanged.
"""

import argparse
import hashlib
import os
import re
import subprocess
import sys
import datetime

_HERE = os.path.dirname(os.path.abspath(__file__))
# Support both a script in the repository root and a script in <repo>/updater/.
# Keep the script directory as the final fallback so a missing checkout produces
# a useful git error instead of silently targeting a developer-specific path.
_REPO_CANDIDATES = (_HERE, os.path.dirname(_HERE))
DEFAULT_REPO_DIR = next(
    (path for path in _REPO_CANDIDATES
     if os.path.exists(os.path.join(path, ".git"))),
    _HERE,
)
DEFAULT_REPO = "jordan1227/improved_to_he"
DEFAULT_REF = "main"
MANIFEST_NAME = "manifest.txt"
MENU_VERSION_FILES = (
    "gamedata/config/text/eng/ui_st_other.xml",
    "gamedata/config/text/rus/ui_st_other.xml",
)
MENU_VERSION_RE = re.compile(
    r'(?P<prefix><string id="st_mode_version"><text>'
    r'NLC Improved: Hard Edition\. Patch:\s*)'
    r'(?P<stamp>\d{2}\.\d{2}\.\d{2}(?:\s+\d{2}:\d{2}\s+МСК)?)'
    r'(?P<suffix>\. STATUS: OPEN BETA</text></string>)'
)
PATCH_STAMP_RE = re.compile(r"^\d{2}\.\d{2}\.\d{2} \d{2}:\d{2} (?:МСК|MSK)$")
# Moscow Standard Time is UTC+03:00 year-round; use a fixed offset so this
# also works with Windows Python installations that lack the IANA tz database.
MOSCOW = datetime.timezone(datetime.timedelta(hours=3), name="MSK")

# What the launcher installs into the player's game folder.
INCLUDE_PREFIXES = ("gamedata/",)
INCLUDE_FILES = ("he_gui.dll",)
# Never shipped automatically: optional packs, spawn sources, dev material.
EXCLUDE_PREFIXES = (
    "Опционально/",
    "all.spawn.improved_3.9.1_ogsr/",
    "all.spawn_NLC.HE/",
    "fl_hook/",
    "docs/",
    "reviews/",
    "old_HE/",
    "tools/",
)
EXCLUDE_FILES = (MANIFEST_NAME, ".gitattributes", ".gitignore", "README.md")

# Optional variants (weapon packs) are copied over gamedata by hand. The build
# ships its own version of part of what they replace, so an update puts the stock
# weapons, hands and watch back and quietly uninstalls the pack. The manifest
# therefore carries, per variant, the list of paths the launcher must leave alone
# while that variant is installed; the variant announces itself with VARIANT_MARKER,
# which it drops into gamedata together with the rest of its files.
# one category per optional folder, each with its own marker, so a weapon pack,
# a weather pack and a bolt pack can be installed at the same time
VARIANT_CATEGORIES = {
    "Опционально/оружие/": "gamedata/variant_weapons.ltx",
    "Опционально/погода/": "gamedata/variant_weather.ltx",
    "Опционально/болтяры/": "gamedata/variant_bolt.ltx",
}
# a pack's file is protected when the build ships the same path, and always for
# scripts: X-Ray addresses a module by bare filename, so a pack script sitting in
# its own subfolder still shadows -- and is shadowed by -- a shipped module.
KEEP_ALWAYS_PREFIXES = ("gamedata/scripts/",)


def git(repo_dir, *args, binary=False):
    out = subprocess.run(
        ["git", "-C", repo_dir] + list(args),
        stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True)
    return out.stdout if binary else out.stdout.decode("utf-8", "replace")


def all_files(repo_dir, rev):
    """Return [(path, blob_sha, size)] for every blob in the tree."""
    raw = git(repo_dir, "ls-tree", "-r", "-l", "-z", rev, binary=True)
    items = []
    for rec in raw.split(b"\x00"):
        if not rec:
            continue
        meta, _, path = rec.partition(b"\t")
        path = path.decode("utf-8")
        mode, otype, sha, size = meta.decode("utf-8").split()
        if otype != "blob":
            continue
        items.append((path, sha, int(size)))
    items.sort(key=lambda it: it[0])
    return items


def tracked_files(repo_dir, rev):
    """Return [(path, blob_sha, size)] for the files the launcher ships."""
    return [it for it in all_files(repo_dir, rev) if wanted(it[0])]


def variant_keep(everything, shipped):
    """-> {variant name: [paths the launcher must not touch]}.

    `everything` is the whole tree, `shipped` the set of paths in the manifest.
    """
    owned = {}
    for path, _, _ in everything:
        for prefix, marker in VARIANT_CATEGORIES.items():
            if not path.startswith(prefix):
                continue
            rest = path[len(prefix):]
            name, slash, rel = rest.partition("/")
            if not slash or not rel.startswith("gamedata/"):
                break
            if rel != marker and rel not in shipped \
                    and not rel.startswith(KEEP_ALWAYS_PREFIXES):
                break
            owned.setdefault(name, []).append(rel)
            break
    for name in owned:
        owned[name] = sorted(set(owned[name]))
    return owned


def wanted(path):
    if path in EXCLUDE_FILES:
        return False
    for pref in EXCLUDE_PREFIXES:
        if path.startswith(pref):
            return False
    if path in INCLUDE_FILES:
        return True
    return any(path.startswith(p) for p in INCLUDE_PREFIXES)


def blob_hashes(repo_dir, shas, progress=None):
    """sha256 of every blob, via one `git cat-file --batch` pass."""
    proc = subprocess.Popen(
        ["git", "-C", repo_dir, "cat-file", "--batch"],
        stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    result = {}
    try:
        total = len(shas)
        for index, sha in enumerate(shas, 1):
            proc.stdin.write((sha + "\n").encode("ascii"))
            proc.stdin.flush()
            header = proc.stdout.readline().decode("ascii").split()
            if len(header) != 3:
                raise RuntimeError("unexpected cat-file header: %r" % header)
            size = int(header[2])
            data = b""
            while len(data) < size:
                chunk = proc.stdout.read(size - len(data))
                if not chunk:
                    raise RuntimeError("short read for blob %s" % sha)
                data += chunk
            proc.stdout.read(1)  # trailing LF
            result[sha] = (hashlib.sha256(data).hexdigest(), size)
            if progress:
                progress(index, total)
    finally:
        proc.stdin.close()
        proc.wait()
    return result


def worktree_hashes(repo_dir, items, progress=None):
    """sha256 of the checked-out files instead of the blobs (mirror hosting)."""
    result = {}
    total = len(items)
    for index, (path, sha, _) in enumerate(items, 1):
        full = os.path.join(repo_dir, path.replace("/", os.sep))
        h = hashlib.sha256()
        size = 0
        with open(full, "rb") as fh:
            while True:
                chunk = fh.read(1 << 20)
                if not chunk:
                    break
                size += len(chunk)
                h.update(chunk)
        result[sha] = (h.hexdigest(), size)
        if progress:
            progress(index, total)
    return result


def update_menu_version(repo_dir, stamp):
    """Update the shared main-menu patch stamp, preserving CP1251 and CRLF."""
    changed = []
    for rel in MENU_VERSION_FILES:
        full = os.path.join(repo_dir, rel.replace("/", os.sep))
        try:
            with open(full, "rb") as fh:
                raw = fh.read()
        except OSError as exc:
            raise SystemExit("cannot update menu version %s: %s" % (rel, exc))
        try:
            text = raw.decode("cp1251")
        except UnicodeDecodeError as exc:
            raise SystemExit("cannot decode %s as Windows-1251: %s" % (rel, exc))
        matches = list(MENU_VERSION_RE.finditer(text))
        if len(matches) != 1:
            raise SystemExit(
                "%s: expected exactly one st_mode_version entry, found %d"
                % (rel, len(matches)))
        updated = MENU_VERSION_RE.sub(
            lambda match: match.group("prefix") + stamp + match.group("suffix"),
            text,
            count=1,
        )
        if updated == text:
            continue
        with open(full, "wb") as fh:
            fh.write(updated.encode("cp1251"))
        changed.append(rel)
    return changed


def build(repo_dir, rev, repo, ref, worktree=False, worktree_paths=(),
          progress=None):
    everything = all_files(repo_dir, rev)
    items = [it for it in everything if wanted(it[0])]
    if not items:
        raise SystemExit("nothing matched the include rules -- wrong repo dir?")
    keep = variant_keep(everything, {p for p, _, _ in items})
    hashes = (worktree_hashes(repo_dir, items, progress=progress) if worktree
              else blob_hashes(repo_dir, [sha for _, sha, _ in items],
                              progress=progress))
    worktree_paths = set(worktree_paths)
    if worktree_paths and not worktree:
        overlay = [it for it in items if it[0] in worktree_paths]
        hashes.update(worktree_hashes(repo_dir, overlay, progress=progress))
    head = git(repo_dir, "rev-parse", "--short", rev).strip()
    stamp = datetime.datetime.now(MOSCOW).strftime("%Y-%m-%d")

    lines = [
        "#nlc-manifest 1",
        "#repo %s" % repo,
        "#ref %s" % ref,
        "#version %s-%s" % (stamp, head),
        "#files %d" % len(items),
    ]
    for name in sorted(keep):
        for rel in keep[name]:
            lines.append("#keep %s %s" % (name, rel))
    total = 0
    for path, sha, _ in items:
        digest, size = hashes[sha]
        total += size
        lines.append("%s %d %s" % (digest, size, path))
    lines.insert(5, "#bytes %d" % total)
    return "\n".join(lines) + "\n", len(items), total


def body_line(line):
    """The part of the manifest --verify compares: files and variant keep lists."""
    return not line.startswith("#") or line.startswith("#keep ")


def progress_bar(done, total):
    """Render a small ASCII progress bar for the hash pass."""
    if getattr(progress_bar, "disabled", False):
        return
    if done != total and done % 64:
        return
    width = 24
    filled = width if total == 0 else int(width * done / total)
    bar = "#" * filled + "-" * (width - filled)
    try:
        print("\r[%s] %d/%d" % (bar, done, total), end="", flush=True)
        if done == total:
            print()
    except OSError:
        progress_bar.disabled = True


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--repo-dir", default=DEFAULT_REPO_DIR)
    ap.add_argument("--rev", default="HEAD")
    ap.add_argument("--repo", default=DEFAULT_REPO)
    ap.add_argument("--ref", default=DEFAULT_REF)
    ap.add_argument("--out")
    ap.add_argument("--prefix", action="append",
                    help="override the include prefixes (repeatable, testing)")
    ap.add_argument("--worktree", action="store_true",
                    help="hash the checked-out files, not the git blobs")
    ap.add_argument("--apply", action="store_true")
    ap.add_argument("--verify", action="store_true")
    ap.add_argument("--no-menu-version", action="store_true",
                    help="do not update the main-menu patch stamp on --apply")
    ap.add_argument("--patch-stamp", metavar="DD.MM.YY_HH:MM_MСК",
                    help="override the Moscow patch stamp used by --apply")
    args = ap.parse_args()

    if args.prefix:
        global INCLUDE_PREFIXES, INCLUDE_FILES
        INCLUDE_PREFIXES = tuple(args.prefix)
        INCLUDE_FILES = ()

    if args.patch_stamp:
        patch_stamp = args.patch_stamp.replace("_", " ")
        if not PATCH_STAMP_RE.fullmatch(patch_stamp):
            ap.error("--patch-stamp must be DD.MM.YY_HH:MM_MСК")
        if patch_stamp.endswith(" MSK"):
            patch_stamp = patch_stamp[:-4] + " МСК"
    else:
        patch_stamp = (datetime.datetime.now(MOSCOW).strftime("%d.%m.%y %H:%M")
                       + " МСК")

    out = args.out or os.path.join(args.repo_dir, MANIFEST_NAME)
    menu_paths = ()
    if args.apply and not args.no_menu_version:
        menu_paths = update_menu_version(args.repo_dir, patch_stamp)
    text, count, total = build(args.repo_dir, args.rev, args.repo, args.ref,
                               worktree=args.worktree,
                               worktree_paths=MENU_VERSION_FILES if menu_paths else (),
                               progress=progress_bar)

    if args.verify:
        if not os.path.exists(out):
            print("MISSING: %s" % out)
            return 1
        with open(out, "rb") as fh:
            old = fh.read().decode("utf-8")
        same = old.replace("\r\n", "\n") == text
        # the version line carries today's date, so compare the file list only
        body_old = [l for l in old.splitlines() if body_line(l)]
        body_new = [l for l in text.splitlines() if body_line(l)]
        if body_old == body_new:
            print("OK: %s -- %d files, %.1f MB%s"
                  % (out, count, total / 1048576.0,
                     "" if same else " (header differs only)"))
            return 0
        print("STALE: %s" % out)
        print("  committed: %d entries, rebuilt: %d entries"
              % (len(body_old), len(body_new)))
        print("  note: verification compares against HEAD; commit the changed files before expecting OK")
        for line in sorted(set(body_new) - set(body_old))[:10]:
            print("  + %s" % line)
        for line in sorted(set(body_old) - set(body_new))[:10]:
            print("  - %s" % line)
        return 1

    if not args.apply:
        print("dry run: %d files, %.1f MB -> %s"
              % (count, total / 1048576.0, out))
        return 0

    with open(out, "wb") as fh:
        fh.write(text.encode("utf-8"))
    suffix = ""
    if menu_paths:
        suffix = "; menu version %s" % patch_stamp.replace("МСК", "MSK")
    print("wrote %s -- %d files, %.1f MB%s"
          % (out, count, total / 1048576.0, suffix))
    return 0


if __name__ == "__main__":
    sys.exit(main())
