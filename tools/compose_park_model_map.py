from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

import numpy as np
from PIL import Image, ImageFilter


TARGET_SIZE = 2560
BLOCKED_FALLBACK_RGB = np.array([148, 151, 145], dtype=np.int16)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest().upper()


def reproject(
    image: Image.Image,
    source_bounds: dict[str, float],
    target_bounds: dict[str, float],
) -> Image.Image:
    source_width, source_height = image.size
    source_x_span = source_bounds["maxX"] - source_bounds["minX"]
    source_z_span = source_bounds["maxZ"] - source_bounds["minZ"]
    target_x_span = target_bounds["maxX"] - target_bounds["minX"]
    target_z_span = target_bounds["maxZ"] - target_bounds["minZ"]

    left = (
        (target_bounds["minX"] - source_bounds["minX"])
        / source_x_span
        * source_width
    )
    right = (
        (target_bounds["maxX"] - source_bounds["minX"])
        / source_x_span
        * source_width
    )
    top = (
        (source_bounds["maxZ"] - target_bounds["maxZ"])
        / source_z_span
        * source_height
    )
    bottom = (
        (source_bounds["maxZ"] - target_bounds["minZ"])
        / source_z_span
        * source_height
    )
    return image.resize(
        (TARGET_SIZE, TARGET_SIZE),
        Image.Resampling.BICUBIC,
        box=(left, top, right, bottom),
    )


def normalize_luminance(rgb: np.ndarray, mask: np.ndarray) -> np.ndarray:
    luminance = (
        rgb[..., 0] * 0.2126
        + rgb[..., 1] * 0.7152
        + rgb[..., 2] * 0.0722
    )
    values = luminance[mask]
    if values.size == 0:
        return np.zeros_like(luminance, dtype=np.float32)
    low, high = np.percentile(values, [5.0, 97.0])
    if high <= low + 1.0:
        high = low + 1.0
    return np.clip((luminance - low) / (high - low), 0.0, 1.0)


def palette(level: np.ndarray, low: tuple[int, int, int], high: tuple[int, int, int]) -> np.ndarray:
    start = np.asarray(low, dtype=np.float32)
    end = np.asarray(high, dtype=np.float32)
    return start[None, None, :] + level[..., None] * (end - start)[None, None, :]


def smooth_height(height: np.ndarray, walkable: np.ndarray) -> np.ndarray:
    valid = walkable.astype(np.float32)
    source = np.where(walkable, height, 0.0).astype(np.float32)
    accumulated = source.copy()
    weights = valid.copy()
    for _ in range(6):
        accumulated = (
            accumulated
            + np.roll(accumulated, 1, axis=0)
            + np.roll(accumulated, -1, axis=0)
            + np.roll(accumulated, 1, axis=1)
            + np.roll(accumulated, -1, axis=1)
        )
        weights = (
            weights
            + np.roll(weights, 1, axis=0)
            + np.roll(weights, -1, axis=0)
            + np.roll(weights, 1, axis=1)
            + np.roll(weights, -1, axis=1)
        )
    return accumulated / np.maximum(weights, 1.0)


def compose(
    collision_image: Path,
    height_path: Path,
    walkable_path: Path,
    visual_mask_path: Path,
    terrain_image: Path,
    output_directory: Path,
) -> dict:
    collision = np.asarray(
        Image.open(collision_image).convert("RGB"),
        dtype=np.uint8,
    )
    collision_metadata_path = collision_image.with_suffix(".json")
    collision_metadata = (
        json.loads(collision_metadata_path.read_text(encoding="utf-8"))
        if collision_metadata_path.exists()
        else {}
    )
    walkable = np.fromfile(walkable_path, dtype=np.uint8).reshape(
        TARGET_SIZE,
        TARGET_SIZE,
    ).astype(bool)
    visual_valid = np.fromfile(
        visual_mask_path,
        dtype=np.uint8,
    ).reshape(
        TARGET_SIZE,
        TARGET_SIZE,
    ).astype(bool)
    height = np.fromfile(height_path, dtype=np.float32).reshape(
        TARGET_SIZE,
        TARGET_SIZE,
    )

    source = Image.open(terrain_image).convert("RGB")
    target_bounds = {
        "minX": 6.190000,
        "maxX": 155.544998,
        "minZ": 8.386999,
        "maxZ": 157.720001,
    }
    projected = source.resize(
        (TARGET_SIZE, TARGET_SIZE),
        Image.Resampling.BICUBIC,
    )
    terrain_rgb = np.asarray(projected, dtype=np.float32)
    visual_rgb = collision.astype(np.float32)

    output = terrain_rgb * 0.94
    output = np.where(
        (visual_valid & walkable)[..., None],
        visual_rgb,
        output,
    )

    smoothed = smooth_height(height, walkable)
    gradient_y, gradient_x = np.gradient(smoothed)
    normal = np.dstack(
        (
            -gradient_x * 32.0,
            np.ones_like(smoothed),
            -gradient_y * 32.0,
        )
    )
    normal /= np.maximum(
        np.linalg.norm(normal, axis=2, keepdims=True),
        1.0e-6,
    )
    light = np.array([-0.42, 0.82, -0.38], dtype=np.float32)
    light /= np.linalg.norm(light)
    shade = 0.92 + 0.10 * np.clip(normal @ light, -0.25, 1.0)
    output *= shade[..., None]

    obstacle_color = np.array([35, 42, 46], dtype=np.float32)
    output[~walkable] = (
        output[~walkable] * 0.28
        + obstacle_color * 0.72
    )

    edge = np.zeros_like(walkable)
    edge[1:, :] |= walkable[1:, :] & ~walkable[:-1, :]
    edge[:-1, :] |= walkable[:-1, :] & ~walkable[1:, :]
    edge[:, 1:] |= walkable[:, 1:] & ~walkable[:, :-1]
    edge[:, :-1] |= walkable[:, :-1] & ~walkable[:, 1:]
    edge_image = Image.fromarray((edge * 255).astype(np.uint8), mode="L")
    edge = np.asarray(edge_image, dtype=np.uint8) > 0
    output[edge] *= np.array([0.62, 0.68, 0.70], dtype=np.float32)

    output = np.clip(output, 0, 255).astype(np.uint8)
    result = Image.fromarray(output, mode="RGB")
    output_directory.mkdir(parents=True, exist_ok=True)
    final_path = output_directory / "park_map_2560.png"
    preview_path = output_directory / "park_map_1024.png"
    hillshade_path = output_directory / "park_height_hillshade_1024.png"
    visual_mask_preview_path = (
        output_directory / "park_visual_coverage_1024.png"
    )
    semantic_preview_path = (
        output_directory / "park_semantic_mask_1024.png"
    )
    result.save(final_path, optimize=True)
    result.resize((1024, 1024), Image.Resampling.LANCZOS).save(
        preview_path,
        optimize=True,
    )

    height_normalized = np.clip(
        (smoothed - np.nanmin(smoothed[walkable]))
        / max(
            np.nanmax(smoothed[walkable]) - np.nanmin(smoothed[walkable]),
            0.01,
        ),
        0.0,
        1.0,
    )
    shade_visual = np.clip(
        (0.35 + 0.65 * np.clip(normal @ light, 0.0, 1.0))
        * (0.55 + 0.45 * height_normalized),
        0.0,
        1.0,
    )
    hillshade = np.clip(shade_visual * 255.0, 0, 255).astype(np.uint8)
    hillshade[~walkable] = 18
    Image.fromarray(hillshade, mode="L").resize(
        (1024, 1024),
        Image.Resampling.LANCZOS,
    ).save(hillshade_path, optimize=True)
    Image.fromarray(
        np.where(visual_valid & walkable, 255, 0).astype(np.uint8),
        mode="L",
    ).resize(
        (1024, 1024),
        Image.Resampling.NEAREST,
    ).save(visual_mask_preview_path, optimize=True)
    semantic = np.zeros((TARGET_SIZE, TARGET_SIZE, 3), dtype=np.uint8)
    semantic[walkable] = np.array([70, 130, 86], dtype=np.uint8)
    semantic[visual_valid & walkable] = np.array(
        [82, 142, 185],
        dtype=np.uint8,
    )
    semantic[~walkable] = np.array([39, 43, 47], dtype=np.uint8)
    Image.fromarray(semantic, mode="RGB").resize(
        (1024, 1024),
        Image.Resampling.NEAREST,
    ).save(semantic_preview_path, optimize=True)

    metadata = {
        "stage": 14,
        "type": "collision_visual_mesh_color",
        "image": final_path.name,
        "preview": preview_path.name,
        "hillshade": hillshade_path.name,
        "visualCoverage": visual_mask_preview_path.name,
        "semanticMask": semantic_preview_path.name,
        "gridWidth": TARGET_SIZE,
        "gridHeight": TARGET_SIZE,
        **target_bounds,
        "minY": 49.001999,
        "maxY": 52.145000,
        "imageNorthAxis": "+Z",
        "walkableCells": int(walkable.sum()),
        "blockedCells": int((~walkable).sum()),
        "visualColorCells": int((visual_valid & walkable).sum()),
        "terrainFallbackCells": int((~visual_valid & walkable).sum()),
        "visualCoverageRatio": float(
            (visual_valid & walkable).sum()
            / max(int(walkable.sum()), 1)
        ),
        "collisionMaterialSha256": sha256(collision_image),
        "terrainCaptureSha256": sha256(terrain_image),
        "heightSha256": sha256(height_path),
        "walkableSha256": sha256(walkable_path),
        "finalSha256": sha256(final_path),
        "capture": collision_metadata,
    }
    (output_directory / "park_map_2560.json").write_text(
        json.dumps(metadata, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    return metadata


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Compose a collision-validated Park map with model colors."
    )
    parser.add_argument("--collision-image", type=Path, required=True)
    parser.add_argument("--height", type=Path, required=True)
    parser.add_argument("--walkable", type=Path, required=True)
    parser.add_argument("--visual-mask", type=Path, required=True)
    parser.add_argument("--terrain-image", type=Path, required=True)
    parser.add_argument("--output-directory", type=Path, required=True)
    args = parser.parse_args()
    metadata = compose(
        args.collision_image,
        args.height,
        args.walkable,
        args.visual_mask,
        args.terrain_image,
        args.output_directory,
    )
    print(json.dumps(metadata, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
