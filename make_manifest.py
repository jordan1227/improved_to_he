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
    python make_manifest.py --changelog-status
    python make_manifest.py --changelog --changelog-days 14

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
CHANGELOG_NAME = "CHANGELOG.md"
CHANGELOG_DEFAULT_DAYS = 14
CHANGELOG_HEAD_RE = re.compile(
    r"^<!-- nlc-changelog-head: ([0-9a-f]{40}) -->\s*\r?\n?",
    re.MULTILINE,
)
CHANGELOG_ENTRY_RE = re.compile(
    r"^<!-- nlc-changelog-commit: ([0-9a-f]{40}) -->$", re.MULTILINE
)
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


def progress_bar(done, total, label="Manifest", step=64):
    """Render a small ASCII progress bar for a long-running pass."""
    if getattr(progress_bar, "disabled", False):
        return
    if done != total and done % step:
        return
    width = 24
    filled = width if total == 0 else int(width * done / total)
    bar = "#" * filled + "-" * (width - filled)
    try:
        prefix = (label + ": ") if label else ""
        print("\r%s[%s] %d/%d" % (prefix, bar, done, total), end="", flush=True)
        if done == total:
            print()
    except OSError:
        progress_bar.disabled = True


def commit_changes(repo_dir, rev):
    """Return [(status, [path, ...])] for one commit."""
    raw = git(repo_dir, "diff-tree", "--root", "--no-commit-id",
              "--name-status", "-r", "-M", "-z", rev, binary=True)
    tokens = raw.split(b"\x00")
    changes = []
    i = 0
    while i < len(tokens):
        if not tokens[i]:
            i += 1
            continue
        status = tokens[i].decode("utf-8", "replace")
        count = 2 if status[:1] in ("R", "C") else 1
        paths = [p.decode("utf-8") for p in tokens[i + 1:i + 1 + count]]
        if len(paths) == count:
            changes.append((status, paths))
        i += 1 + count
    return changes


def commit_info(repo_dir, rev):
    """Return metadata and changed paths for a commit."""
    raw = git(repo_dir, "show", "-s",
              "--format=%H%x00%P%x00%aI%x00%B", rev)
    sha, parents, authored, message = raw.split("\x00", 3)
    return {
        "sha": sha,
        "short": sha[:7],
        "parent": parents.split()[0] if parents.split() else "",
        "authored": authored,
        "message": message.strip(),
        "changes": commit_changes(repo_dir, sha),
    }


def is_ancestor(repo_dir, older, newer):
    proc = subprocess.run(
        ["git", "-C", repo_dir, "merge-base", "--is-ancestor", older, newer],
        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
    )
    return proc.returncode == 0


def recent_changelog_base(repo_dir, target, days):
    if days <= 0:
        raise SystemExit("--changelog-days must be greater than zero")
    print("Finding commits from the last %d days..." % days)
    recent = git(repo_dir, "rev-list", "--first-parent", "--reverse",
                 "--since=%d days ago" % days, target).splitlines()
    if recent:
        return commit_info(repo_dir, recent[0])["parent"]
    return target


def changelog_commits(repo_dir, target, since=None, changelog_text="", days=CHANGELOG_DEFAULT_DAYS):
    """Return (base, all_commits, payload_commits, ignored_commits)."""
    target_info = commit_info(repo_dir, target)
    marker = CHANGELOG_HEAD_RE.search(changelog_text)
    if since:
        base = since
    else:
        window_base = recent_changelog_base(repo_dir, target_info["sha"], days)
        if marker:
            base = marker.group(1)
            # If the marker is newer than the default window, scan from the
            # older window boundary so missing historical entries are filled
            # without duplicating entries already present in CHANGELOG.md.
            if window_base and is_ancestor(repo_dir, window_base, base):
                base = window_base
        else:
            base = window_base
    if base and not is_ancestor(repo_dir, base, target_info["sha"]):
        raise SystemExit(
            "changelog base %s is not an ancestor of %s; use --changelog-since"
            % (base, target_info["short"]))
    revspec = "%s..%s" % (base, target_info["sha"]) if base else target_info["sha"]
    raw = git(repo_dir, "rev-list", "--first-parent", "--reverse", revspec)
    all_commits = [line for line in raw.splitlines() if line]
    infos = []
    if all_commits:
        print("Scanning changelog commits...")
        total = len(all_commits)
        for index, sha in enumerate(all_commits, 1):
            infos.append(commit_info(repo_dir, sha))
            progress_bar(index, total, label="Changelog", step=16)
    else:
        print("Changelog: no commits to scan")
    payload = [info for info in infos
               if any(wanted(path) for _, paths in info["changes"] for path in paths)]
    ignored = [info for info in infos if info not in payload]
    return base, infos, payload, ignored


def format_change(status, paths):
    if len(paths) >= 2:
        return "%s %s -> %s" % (status, paths[0], paths[1])
    return "%s %s" % (status, paths[0])


def format_changelog_entry(info, repo):
    try:
        authored = datetime.datetime.fromisoformat(
            info["authored"].replace("Z", "+00:00"))
        stamp = authored.astimezone(MOSCOW).strftime("%d.%m.%y %H:%M") + " МСК"
    except ValueError:
        stamp = info["authored"]
    message_lines = info["message"].splitlines()
    subject = message_lines[0].strip() if message_lines else "(no commit subject)"
    body = "\n".join(message_lines[1:]).strip()
    url = "https://github.com/%s/commit/%s" % (repo, info["sha"])
    game_changes = []
    other_changes = []
    for status, paths in info["changes"]:
        target = game_changes if any(wanted(path) for path in paths) else other_changes
        target.append(format_change(status, paths))
    lines = [
        "<!-- nlc-changelog-commit: %s -->" % info["sha"],
        "## %s - %s" % (stamp, subject),
        "",
        "Commit: [%s](%s)" % (info["short"], url),
        "",
        "### Description",
        "",
        body or "_(No additional description.)_",
        "",
        "### Game files changed",
        "",
    ]
    lines.extend("- %s" % change for change in game_changes)
    if not game_changes:
        lines.append("- _(none)_")
    if other_changes:
        lines.extend(["", "### Other repository files changed", ""])
        lines.extend("- %s" % change for change in other_changes)
    return "\n".join(lines)


def read_changelog(path):
    if not os.path.exists(path):
        return ""
    with open(path, "r", encoding="utf-8") as fh:
        return fh.read()


def write_changelog(repo_dir, repo, target, since=None, out=None, days=CHANGELOG_DEFAULT_DAYS):
    out = out or os.path.join(repo_dir, CHANGELOG_NAME)
    old = read_changelog(out)
    base, all_commits, payload, ignored = changelog_commits(
        repo_dir, target, since=since, changelog_text=old, days=days)
    existing = set(CHANGELOG_ENTRY_RE.findall(old))
    new_infos = [info for info in payload if info["sha"] not in existing]
    if not new_infos:
        print("changelog is up to date -- %d maintenance commits ignored" % len(ignored))
        return 0
    entries = [format_changelog_entry(info, repo) for info in reversed(new_infos)]
    marker_sha = payload[-1]["sha"]
    content = CHANGELOG_HEAD_RE.sub("", old, count=1).lstrip()
    if not content:
        content = "# NLC Improved changelog\n"
    if not content.startswith("# NLC Improved changelog"):
        content = "# NLC Improved changelog\n\n" + content
    header, _, rest = content.partition("\n")
    blocks = [header, "", "\n\n".join(entries)]
    if rest.strip():
        blocks.extend(["", rest.strip()])
    final = "<!-- nlc-changelog-head: %s -->\n\n%s\n" % (
        marker_sha, "\n".join(blocks))
    with open(out, "w", encoding="utf-8", newline="\n") as fh:
        fh.write(final)
    print("wrote %s -- %d entries; %d maintenance commits ignored"
          % (out, len(new_infos), len(ignored)))
    return 0


def changelog_status(repo_dir, target, since=None, out=None, days=CHANGELOG_DEFAULT_DAYS):
    out = out or os.path.join(repo_dir, CHANGELOG_NAME)
    old = read_changelog(out)
    base, all_commits, payload, ignored = changelog_commits(
        repo_dir, target, since=since, changelog_text=old, days=days)
    existing = set(CHANGELOG_ENTRY_RE.findall(old))
    missing = [info for info in payload if info["sha"] not in existing]
    marker = CHANGELOG_HEAD_RE.search(old)
    print("changelog target: %s" % commit_info(repo_dir, target)["sha"][:7])
    print("changelog marker: %s" % (marker.group(1)[:7] if marker else "none"))
    if not since:
        print("note: default window is the last %d days; --changelog-since overrides it" % days)
    print("commits scanned: %d" % len(all_commits))
    print("missing entries: %d" % len(missing))
    print("maintenance commits ignored: %d" % len(ignored))
    for info in missing:
        subject = info["message"].splitlines()[0] if info["message"] else "(no subject)"
        safe_subject = subject.encode("ascii", "replace").decode("ascii")
        print("  %s %s" % (info["short"], safe_subject))
    return 0


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
    ap.add_argument("--changelog", action="store_true",
                    help="generate/catch up CHANGELOG.md from Git history")
    ap.add_argument("--changelog-status", action="store_true",
                    help="show missing changelog entries without writing")
    ap.add_argument("--changelog-since", metavar="REV",
                    help="history base revision for the first changelog run")
    ap.add_argument("--changelog-days", type=int, default=CHANGELOG_DEFAULT_DAYS,
                    help="number of recent days to include on a first run (default: %(default)s)")
    ap.add_argument("--changelog-out", metavar="PATH",
                    help="override the CHANGELOG.md output path")
    args = ap.parse_args()

    if args.prefix:
        global INCLUDE_PREFIXES, INCLUDE_FILES
        INCLUDE_PREFIXES = tuple(args.prefix)
        INCLUDE_FILES = ()

    if args.changelog and args.changelog_status:
        ap.error("--changelog and --changelog-status are mutually exclusive")
    if args.changelog or args.changelog_status:
        if args.apply or args.verify:
            ap.error("changelog actions cannot be combined with --apply or --verify")
        changelog_out = args.changelog_out or os.path.join(
            args.repo_dir, CHANGELOG_NAME)
        if args.changelog_status:
            print("Starting changelog status scan...")
            return changelog_status(args.repo_dir, args.rev,
                                    since=args.changelog_since, out=changelog_out,
                                    days=args.changelog_days)
        print("Starting changelog generation...")
        return write_changelog(args.repo_dir, args.repo, args.rev,
                               since=args.changelog_since, out=changelog_out,
                               days=args.changelog_days)

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
    print("Starting manifest build...")
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
