#!/usr/bin/env python3
"""Add the standalone DildoFloor item using captured five-quality images."""

from __future__ import annotations

import argparse
import json
import shutil
from pathlib import Path

from PIL import Image


QUALITIES = ("white", "green", "purple", "gold", "black")
ITEM_ID = "217-adult-011-mitemdildo"
SOURCE_ID = "207-adult-011-mitemdildo"
CROP = (0.42, 0.27, 0.16, 0.32)


def load_json(path: Path) -> dict:
    with path.open("r", encoding="utf-8") as handle:
        return json.load(handle)


def write_json(path: Path, data: dict) -> None:
    temporary = path.with_suffix(path.suffix + ".tmp")
    with temporary.open("w", encoding="utf-8", newline="\n") as handle:
        json.dump(data, handle, ensure_ascii=False, indent=2)
        handle.write("\n")
    temporary.replace(path)


def crop_box(
    width: int,
    height: int,
    normalized: tuple[float, float, float, float],
) -> tuple[int, int, int, int]:
    nx, ny, nw, nh = normalized
    left = round(nx * width)
    top = round(ny * height)
    right = round((nx + nw) * width)
    bottom = round((ny + nh) * height)
    return left, top, right, bottom


def image_record() -> dict:
    return {
        "id": ITEM_ID,
        "display_name": ITEM_ID,
        "name": "成人用品·假阳具",
        "series": "成人用品",
        "theme": "开放",
        "description": "独立放置型成人用品，可部署到地面并使用。",
        "reference_price": 0,
        "categories": ["prop"],
        "categories_suggested": ["prop"],
        "occupancy": {
            "label": "1x2",
            "width": 1,
            "height": 2,
        },
        "crop": {
            "normalized": list(CROP),
            "source_size": [640, 640],
            "aspect_ratio": 0.5,
        },
        "images": {
            quality: f"crops/{quality}/{ITEM_ID}.png"
            for quality in QUALITIES
        },
        "stats": {},
        "item_type": 1,
        "drop_item_type": 10000,
        "standalone_model": True,
        "complete": True,
    }


def add_to_catalog(path: Path) -> None:
    catalog = load_json(path)
    catalog.setdefault("items", {})[ITEM_ID] = image_record()
    series_order = catalog.setdefault("series_order", [])
    if "成人用品" not in series_order:
        series_order.insert(1, "成人用品")
    write_json(path, catalog)


def copy_sources(source_root: Path) -> None:
    for quality in QUALITIES:
        source = source_root / quality / f"{SOURCE_ID}.png"
        destination = source_root / quality / f"{ITEM_ID}.png"
        if not source.is_file():
            raise FileNotFoundError(source)
        shutil.copy2(source, destination)


def write_crops(source_root: Path, output_root: Path) -> None:
    for quality in QUALITIES:
        source = source_root / quality / f"{ITEM_ID}.png"
        destination = output_root / quality / f"{ITEM_ID}.png"
        destination.parent.mkdir(parents=True, exist_ok=True)
        with Image.open(source) as image:
            image = image.convert("RGBA")
            cropped = image.crop(
                crop_box(image.width, image.height, CROP)
            )
            cropped.save(destination, "PNG", compress_level=6)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-root", required=True, type=Path)
    parser.add_argument("--catalog", required=True, action="append", type=Path)
    parser.add_argument("--crop-root", required=True, action="append", type=Path)
    args = parser.parse_args()

    copy_sources(args.source_root)
    for catalog in args.catalog:
        add_to_catalog(catalog)
    for crop_root in args.crop_root:
        write_crops(args.source_root, crop_root)
    print(f"added={ITEM_ID} catalogs={len(args.catalog)} crops={len(args.crop_root)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
