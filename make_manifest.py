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
    python make_manifest.py --apply --rev HEAD --repo-dir D:\impr\impr\to_git
"""

import argparse
import hashlib
import os
import subprocess
import sys
import datetime

_HERE = os.path.dirname(os.path.abspath(__file__))
# repo root when this script lives in <repo>/updater/, dev path otherwise
DEFAULT_REPO_DIR = (os.path.dirname(_HERE)
                   if os.path.isdir(os.path.join(os.path.dirname(_HERE), ".git"))
                   else r"D:\impr\impr\to_git")
DEFAULT_REPO = "jordan1227/improved_to_he"
DEFAULT_REF = "main"
MANIFEST_NAME = "manifest.txt"

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


def blob_hashes(repo_dir, shas):
    """sha256 of every blob, via one `git cat-file --batch` pass."""
    proc = subprocess.Popen(
        ["git", "-C", repo_dir, "cat-file", "--batch"],
        stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    result = {}
    try:
        for sha in shas:
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
    finally:
        proc.stdin.close()
        proc.wait()
    return result


def worktree_hashes(repo_dir, items):
    """sha256 of the checked-out files instead of the blobs (mirror hosting)."""
    result = {}
    for path, sha, _ in items:
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
    return result


def build(repo_dir, rev, repo, ref, worktree=False):
    everything = all_files(repo_dir, rev)
    items = [it for it in everything if wanted(it[0])]
    if not items:
        raise SystemExit("nothing matched the include rules -- wrong repo dir?")
    keep = variant_keep(everything, {p for p, _, _ in items})
    hashes = (worktree_hashes(repo_dir, items) if worktree
              else blob_hashes(repo_dir, [sha for _, sha, _ in items]))
    head = git(repo_dir, "rev-parse", "--short", rev).strip()
    stamp = datetime.datetime.now().strftime("%Y-%m-%d")

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
    args = ap.parse_args()

    if args.prefix:
        global INCLUDE_PREFIXES, INCLUDE_FILES
        INCLUDE_PREFIXES = tuple(args.prefix)
        INCLUDE_FILES = ()

    out = args.out or os.path.join(args.repo_dir, MANIFEST_NAME)
    text, count, total = build(args.repo_dir, args.rev, args.repo, args.ref,
                               worktree=args.worktree)

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
    print("wrote %s -- %d files, %.1f MB" % (out, count, total / 1048576.0))
    return 0


if __name__ == "__main__":
    sys.exit(main())
