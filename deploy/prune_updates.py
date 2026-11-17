#!/usr/bin/env python3
import argparse
import json
import os
import shutil
import sys
from datetime import datetime, timedelta, timezone
from pathlib import Path


ROOT = Path("/srv/efm/public")
DEV_ROOT = ROOT / "dev"
STAGING = Path("/srv/efm/staging")
MAX_OBJECT_BYTES = 20 * 1024 * 1024 * 1024
MIN_FREE_BYTES = 8 * 1024 * 1024 * 1024
RETAIN = {
    "dev": 20,
    "stable": 10,
}
OBJECT_GRACE = timedelta(days=30)


def size_of(path: Path) -> int:
    total = 0
    for item in path.rglob("*"):
        if item.is_file() and not item.is_symlink():
            try:
                total += item.stat().st_size
            except FileNotFoundError:
                pass
    return total


def channel_manifests(channel: str) -> list[Path]:
    directory = (
        DEV_ROOT / "releases" / "dev"
        if channel == "dev"
        else ROOT / "releases" / "stable"
    )
    if not directory.is_dir():
        return []
    return sorted(
        directory.glob("*.json"),
        key=lambda item: item.stat().st_mtime,
        reverse=True,
    )


def object_hashes(manifest_path: Path) -> set[str]:
    try:
        data = json.loads(manifest_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return set()
    return {
        str(record["sha256"]).lower()
        for record in data.get("files", [])
        if record.get("sha256")
    }


def remove_file(path: Path, dry_run: bool) -> None:
    print(f"{'would remove' if dry_run else 'remove'} {path}")
    if not dry_run:
        path.unlink(missing_ok=True)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    if not ROOT.is_dir():
        print(f"Missing update root: {ROOT}", file=sys.stderr)
        return 1

    retained_manifests: list[Path] = []
    for channel, keep_count in RETAIN.items():
        manifests = channel_manifests(channel)
        retained_manifests.extend(manifests[:keep_count])
        for stale in manifests[keep_count:]:
            remove_file(stale, args.dry_run)

    for channel in RETAIN:
        current = (
            DEV_ROOT / "channels" / "dev.json"
            if channel == "dev"
            else ROOT / "channels" / "stable.json"
        )
        if current.is_file():
            retained_manifests.append(current)

    referenced = set()
    for manifest in retained_manifests:
        referenced.update(object_hashes(manifest))

    cutoff = datetime.now(timezone.utc) - OBJECT_GRACE
    object_roots = [ROOT / "objects", DEV_ROOT / "objects"]
    for objects in object_roots:
        if not objects.is_dir():
            continue
        for obj in objects.iterdir():
            if not obj.is_file() or obj.name.lower() in referenced:
                continue
            modified = datetime.fromtimestamp(obj.stat().st_mtime, timezone.utc)
            if modified < cutoff:
                remove_file(obj, args.dry_run)

    if STAGING.is_dir():
        cutoff_staging = datetime.now(timezone.utc) - timedelta(days=2)
        for staged in STAGING.iterdir():
            modified = datetime.fromtimestamp(staged.stat().st_mtime, timezone.utc)
            if modified >= cutoff_staging:
                continue
            print(f"{'would remove' if args.dry_run else 'remove'} {staged}")
            if not args.dry_run:
                if staged.is_dir() and not staged.is_symlink():
                    shutil.rmtree(staged)
                else:
                    staged.unlink(missing_ok=True)

    object_bytes = sum(size_of(path) for path in object_roots if path.is_dir())
    volume = shutil.disk_usage(ROOT)
    print(
        f"objects={object_bytes} bytes "
        f"volume_used={volume.used} bytes "
        f"volume_free={volume.free} bytes "
        f"referenced={len(referenced)}"
    )

    if object_bytes > MAX_OBJECT_BYTES:
        print("EFM object quota exceeded.", file=sys.stderr)
        return 2
    if volume.free < MIN_FREE_BYTES:
        print("Filesystem free-space safety reserve exceeded.", file=sys.stderr)
        return 3
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
