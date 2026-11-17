#!/usr/bin/env python3
"""Prune Manaka's equipment catalog and normalize in-game series names."""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path


COAT_ID = "211-clothes-001-mclothesrecordcoat"
BASIC_ANAL_PLUG_ID = "007-costume-007-rcostumeanalplag"
REMOVED_ADULT_IDS = {
    "203-adult-004-madultgoodsrecordkeyhandcuff",
    "204-adult-005-madultgoodsrecordhundcufftimer",
}
EXPECTED_ITEM_COUNT = 208

COSPLAY_SERIES = {
    "00": "通用头部",
    "01": "通用上半身",
    "02": "通用腿部",
    "03": "通用性器",
    "04": "邪教修女",
    "05": "狱警",
    "06": "乳胶小马",
    "07": "兔女郎",
    "08": "野兽",
    "09": "战术女仆",
    "10": "魅魔",
    "11": "奶牛",
    "12": "忍者",
    "13": "猫娘",
    "14": "兔女郎",
    "15": "Chic西装",
    "16": "奢华西装",
    "17": "毛衣",
    "18": "奢华西装",
    "19": "校园连帽",
    "20": "校园辣妹",
}

SERIES_ORDER = [
    "基础服装",
    "成人用品",
    *dict.fromkeys(COSPLAY_SERIES.values()),
]


def load_json(path: Path) -> dict:
    with path.open("r", encoding="utf-8") as handle:
        return json.load(handle)


def write_json(path: Path, data: dict) -> None:
    temporary = path.with_suffix(path.suffix + ".tmp")
    with temporary.open("w", encoding="utf-8", newline="\n") as handle:
        json.dump(data, handle, ensure_ascii=False, indent=2)
        handle.write("\n")
    temporary.replace(path)


def series_for_id(item_id: str) -> str | None:
    if "-cosplay-" in item_id:
        match = re.search(r"-cosplay-(\d{2})-", item_id)
        return COSPLAY_SERIES.get(match.group(1)) if match else None
    if "-adult-" in item_id:
        return None if item_id in REMOVED_ADULT_IDS else "成人用品"
    if item_id == COAT_ID:
        return "基础服装"
    if "-costume-" in item_id and item_id != BASIC_ANAL_PLUG_ID:
        return "基础服装"
    return None


def rename_record(record: dict, series: str) -> None:
    name = str(record.get("name", "")).strip()
    leaf = name.split("·", 1)[1] if "·" in name else name
    record["series"] = series
    record["name"] = f"{series}·{leaf}"


def prune_catalog(path: Path) -> tuple[set[str], int, int]:
    catalog = load_json(path)
    source_items = catalog.get("items", {})
    kept: dict[str, dict] = {}
    removed_ids: set[str] = set()

    for item_id, record in source_items.items():
        series = series_for_id(item_id)
        if series is None:
            removed_ids.add(item_id)
            continue
        copied = dict(record)
        rename_record(copied, series)
        kept[item_id] = copied

    catalog["items"] = kept
    catalog["series_order"] = SERIES_ORDER
    write_json(path, catalog)
    return removed_ids, len(source_items), len(kept)


def prune_asset_crops(root: Path, removed_ids: set[str]) -> int:
    removed = 0
    if not root.is_dir():
        return removed
    for image in root.glob("*/*.png"):
        if image.stem in removed_ids:
            image.unlink()
            removed += 1
    return removed


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--catalog", required=True, type=Path, action="append")
    parser.add_argument("--asset-root", required=True, type=Path, action="append")
    args = parser.parse_args()

    removed_ids: set[str] = set()
    for catalog_path in args.catalog:
        removed, before, after = prune_catalog(catalog_path)
        removed_ids.update(removed)
        print(f"{catalog_path}: {before} -> {after} items")
        if after != EXPECTED_ITEM_COUNT:
            raise RuntimeError(
                f"Expected {EXPECTED_ITEM_COUNT} kept items in "
                f"{catalog_path}, observed {after}"
            )

    for asset_root in args.asset_root:
        removed = prune_asset_crops(asset_root, removed_ids)
        print(f"{asset_root}: removed {removed} crop(s)")

    print("removed:", ", ".join(sorted(removed_ids)))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
