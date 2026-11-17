from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

import numpy as np
from PIL import Image, ImageFilter


PARK_BOUNDS = {
    "minX": 6.190000,
    "maxX": 155.544998,
    "minY": 49.001999,
    "maxY": 52.145000,
    "minZ": 8.386999,
    "maxZ": 157.720001,
}


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest().upper()


def load_mask(path: Path, size: int, output_size: int) -> np.ndarray:
    mask = np.fromfile(path, dtype=np.uint8).reshape(size, size)
    mask = np.where(mask > 0, 255, 0).astype(np.uint8)
    image = Image.fromarray(mask.astype(np.uint8), mode="L").resize(
        (output_size, output_size),
        Image.Resampling.LANCZOS,
    )
    return np.asarray(image, dtype=np.uint8) >= 96


def load_image(path: Path, output_size: int) -> np.ndarray:
    return np.asarray(
        Image.open(path).convert("RGB").resize(
            (output_size, output_size),
            Image.Resampling.LANCZOS,
        ),
        dtype=np.float32,
    )


def grade_terrain(
    terrain: np.ndarray,
    walkable: np.ndarray,
    output_size: int,
) -> np.ndarray:
    luminance = (
        terrain[..., 0] * 0.2126
        + terrain[..., 1] * 0.7152
        + terrain[..., 2] * 0.0722
    )
    low, high = np.percentile(luminance[walkable], [7.0, 93.0])
    level = np.clip(
        (luminance - low) / max(float(high - low), 1.0),
        0.0,
        1.0,
    )

    # A muted, warm-green wash keeps the source texture while separating the
    # park from the path and obstacle layers.
    tint = np.array([0.64, 0.71, 0.55], dtype=np.float32)
    graded = tint[None, None, :] * (145.0 + level[..., None] * 72.0)
    chroma = terrain - luminance[..., None]
    graded += chroma * 0.20
    graded = np.clip(graded, 0.0, 255.0)
    image = Image.fromarray(graded.astype(np.uint8), mode="RGB")
    return np.asarray(
        image.filter(ImageFilter.GaussianBlur(radius=0.45)),
        dtype=np.float32,
    )


def grade_models(model: np.ndarray, visual: np.ndarray) -> np.ndarray:
    values = model[visual]
    if values.size:
        luminance = (
            values[..., 0] * 0.2126
            + values[..., 1] * 0.7152
            + values[..., 2] * 0.0722
        )
        low, high = np.percentile(luminance, [10.0, 90.0])
    else:
        low, high = 24.0, 88.0
    if high <= low + 1.0:
        high = low + 1.0

    source_luminance = (
        model[..., 0] * 0.2126
        + model[..., 1] * 0.7152
        + model[..., 2] * 0.0722
    )
    level = np.clip(
        (source_luminance - low) / (high - low),
        0.0,
        1.0,
    )
    chroma = model - source_luminance[..., None]
    chroma *= 0.72
    hinted = source_luminance[..., None] + chroma

    # Preserve the material hue, lift the nearly black road palette, and add a
    # restrained stone tint so roads remain readable at README size.
    lifted = hinted * 0.42 + level[..., None] * 154.0
    stone = np.array([170.0, 157.0, 132.0], dtype=np.float32)
    lifted = lifted * 0.72 + stone[None, None, :] * 0.28
    return np.clip(lifted, 0.0, 255.0)


def outline(binary: np.ndarray) -> np.ndarray:
    edge = np.zeros_like(binary, dtype=bool)
    edge[1:, :] |= binary[1:, :] != binary[:-1, :]
    edge[:-1, :] |= binary[:-1, :] != binary[1:, :]
    edge[:, 1:] |= binary[:, 1:] != binary[:, :-1]
    edge[:, :-1] |= binary[:, :-1] != binary[:, 1:]
    return edge


def style(
    terrain_path: Path,
    model_path: Path,
    visual_mask_path: Path,
    walkable_path: Path,
    output_directory: Path,
    source_size: int,
    output_size: int,
) -> dict:
    terrain = grade_terrain(
        load_image(terrain_path, output_size),
        load_mask(walkable_path, source_size, output_size),
        output_size,
    )
    model = load_image(model_path, output_size)
    visual = load_mask(visual_mask_path, source_size, output_size)
    walkable = load_mask(walkable_path, source_size, output_size)
    model = grade_models(model, visual)

    output = terrain.copy()
    output[visual & walkable] = model[visual & walkable]

    # Blocked terrain remains recognizable instead of becoming a flat black
    # mask, while the dark teal keeps trees and walls visually subordinate.
    blocked = ~walkable
    blocked_color = np.array([34.0, 47.0, 48.0], dtype=np.float32)
    output[blocked] = (
        output[blocked] * 0.43
        + blocked_color[None, :] * 0.57
    )

    model_edge = outline(visual & walkable)
    walkable_edge = outline(walkable)
    output[model_edge] = (
        output[model_edge] * 0.56
        + np.array([233.0, 221.0, 193.0]) * 0.44
    )
    output[walkable_edge] = (
        output[walkable_edge] * 0.56
        + np.array([216.0, 205.0, 178.0]) * 0.44
    )

    output = np.clip(output, 0.0, 255.0).astype(np.uint8)
    result = Image.fromarray(output, mode="RGB")
    output_directory.mkdir(parents=True, exist_ok=True)
    png_path = output_directory / f"park_map_readme_{output_size}.png"
    webp_path = output_directory / f"park_map_readme_{output_size}.webp"

    quantized = result.quantize(
        colors=256,
        method=Image.Quantize.MEDIANCUT,
        dither=Image.Dither.NONE,
    )
    quantized.save(png_path, optimize=True)
    result.save(webp_path, format="WEBP", quality=88, method=6)

    metadata = {
        "type": "park_readme_minimap",
        "stage": 14,
        "width": output_size,
        "height": output_size,
        "sourceWidth": source_size,
        "sourceHeight": source_size,
        "image": "park-map.webp",
        "imageNorthAxis": "+Z",
        **PARK_BOUNDS,
        "png": png_path.name,
        "webp": webp_path.name,
        "pngSha256": sha256(png_path),
        "webpSha256": sha256(webp_path),
        "walkableCells": int(walkable.sum()),
        "modelCells": int((visual & walkable).sum()),
        "modelCoverageRatio": float(
            (visual & walkable).sum() / max(int(walkable.sum()), 1)
        ),
        "sources": {
            "terrain": {
                "name": terrain_path.name,
                "sha256": sha256(terrain_path),
            },
            "model": {
                "name": model_path.name,
                "sha256": sha256(model_path),
            },
            "visualMask": {
                "name": visual_mask_path.name,
                "sha256": sha256(visual_mask_path),
            },
            "walkable": {
                "name": walkable_path.name,
                "sha256": sha256(walkable_path),
            },
        },
    }
    (output_directory / f"park_map_readme_{output_size}.json").write_text(
        json.dumps(metadata, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    return metadata


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Create a compact, high-contrast README Park minimap."
    )
    parser.add_argument("--terrain-image", type=Path, required=True)
    parser.add_argument("--model-image", type=Path, required=True)
    parser.add_argument("--visual-mask", type=Path, required=True)
    parser.add_argument("--walkable", type=Path, required=True)
    parser.add_argument("--output-directory", type=Path, required=True)
    parser.add_argument("--source-size", type=int, default=2560)
    parser.add_argument("--size", type=int, default=1024)
    args = parser.parse_args()
    metadata = style(
        args.terrain_image,
        args.model_image,
        args.visual_mask,
        args.walkable,
        args.output_directory,
        args.source_size,
        args.size,
    )
    print(json.dumps(metadata, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
