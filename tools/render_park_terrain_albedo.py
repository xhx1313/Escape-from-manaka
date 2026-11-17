from __future__ import annotations

import argparse
import json
from pathlib import Path

import numpy as np
import UnityPy
from PIL import Image


DEFAULT_BOUNDS = {
    "minX": 6.19,
    "maxX": 155.544998,
    "minZ": 8.386999,
    "maxZ": 157.720001,
}


def find_park_terrain(environment: UnityPy.Environment):
    for entry in environment.objects:
        if entry.type.name != "Terrain":
            continue
        terrain = entry.read()
        game_object = terrain.m_GameObject.read()
        terrain_data = terrain.m_TerrainData.read()
        if (
            game_object.m_Name == "Terrain"
            and game_object.m_Layer == 20
            and terrain_data.m_Name == "New Terrain"
        ):
            return terrain, game_object, terrain_data
    raise RuntimeError("Could not locate the Park TerrainData.")


def sample_tiled(image: np.ndarray, u: np.ndarray, v: np.ndarray) -> np.ndarray:
    height, width = image.shape[:2]
    x = np.mod(u, 1.0) * width
    y = np.mod(v, 1.0) * height
    x0 = np.floor(x).astype(np.int32)
    y0 = np.floor(y).astype(np.int32)
    x1 = np.mod(x0 + 1, width)
    y1 = np.mod(y0 + 1, height)
    fx = (x - x0)[..., None]
    fy = (y - y0)[..., None]
    top = (
        image[y0, x0].astype(np.float32) * (1.0 - fx)
        + image[y0, x1].astype(np.float32) * fx
    )
    bottom = (
        image[y1, x0].astype(np.float32) * (1.0 - fx)
        + image[y1, x1].astype(np.float32) * fx
    )
    return top * (1.0 - fy) + bottom * fy


def render_terrain(
    game_path: Path,
    output_directory: Path,
    size: int,
    bounds: dict[str, float],
) -> dict:
    environment = UnityPy.load(str(game_path))
    terrain, game_object, terrain_data = find_park_terrain(environment)
    splat = terrain_data.m_SplatDatabase
    layers = [reference.read() for reference in splat.m_TerrainLayers]
    alpha_textures = [
        reference.read() for reference in splat.m_AlphaTextures
    ]
    if len(alpha_textures) != 2 or len(layers) not in (5, 8):
        raise RuntimeError(
            f"Unsupported Park splat layout: "
            f"{len(layers)} layers, {len(alpha_textures)} alpha textures."
        )

    world_x = (
        bounds["minX"]
        + (np.arange(size, dtype=np.float32) + 0.5)
        / size
        * (bounds["maxX"] - bounds["minX"])
    )
    world_z = (
        bounds["maxZ"]
        - (np.arange(size, dtype=np.float32) + 0.5)
        / size
        * (bounds["maxZ"] - bounds["minZ"])
    )
    xx, zz = np.meshgrid(world_x, world_z)

    height_scale = terrain_data.m_Heightmap.m_Scale
    terrain_min_x = 0.0
    terrain_min_z = 0.0
    terrain_width = height_scale.x * (
        terrain_data.m_Heightmap.m_Resolution - 1
    )
    terrain_depth = height_scale.z * (
        terrain_data.m_Heightmap.m_Resolution - 1
    )
    terrain_u = np.clip(
        (xx - terrain_min_x) / terrain_width,
        0.0,
        1.0,
    )
    terrain_v = np.clip(
        (zz - terrain_min_z) / terrain_depth,
        0.0,
        1.0,
    )

    alpha0 = np.asarray(alpha_textures[0].image.convert("RGBA"))
    alpha1 = np.asarray(alpha_textures[1].image.convert("RGBA"))
    alpha_resolution = alpha0.shape[0]
    alpha_x = np.clip(
        (terrain_u * (alpha_resolution - 1)).astype(np.int32),
        0,
        alpha_resolution - 1,
    )
    alpha_y = np.clip(
        (terrain_v * (alpha_resolution - 1)).astype(np.int32),
        0,
        alpha_resolution - 1,
    )
    weights = np.stack(
        [
            alpha0[alpha_y, alpha_x, 0],
            alpha0[alpha_y, alpha_x, 1],
            alpha0[alpha_y, alpha_x, 2],
            alpha0[alpha_y, alpha_x, 3],
            alpha1[alpha_y, alpha_x, 0],
            alpha1[alpha_y, alpha_x, 1],
            alpha1[alpha_y, alpha_x, 2],
            alpha1[alpha_y, alpha_x, 3],
        ],
        axis=-1,
    ).astype(np.float32)
    weights /= np.maximum(weights.sum(axis=-1, keepdims=True), 1.0)

    output = np.zeros((size, size, 3), dtype=np.float32)
    layer_coverage = []
    for index, layer in enumerate(layers):
        texture = layer.m_DiffuseTexture.read()
        diffuse = np.asarray(texture.image.convert("RGB"))
        tile_size = layer.m_TileSize
        tile_offset = layer.m_TileOffset
        u = (
            (xx - terrain_min_x) / max(abs(tile_size.x), 1.0e-6)
            - tile_offset.x
        )
        v = (
            (zz - terrain_min_z) / max(abs(tile_size.y), 1.0e-6)
            - tile_offset.y
        )
        layer_rgb = sample_tiled(diffuse, u, v)
        layer_weight = weights[..., index]
        output += layer_rgb * layer_weight[..., None]
        layer_coverage.append(
            {
                "index": index,
                "name": layer.m_Name,
                "texture": texture.m_Name,
                "tileSize": [float(tile_size.x), float(tile_size.y)],
                "weightAverage": float(layer_weight.mean()),
                "weightCoverage": float((layer_weight > 0.01).mean()),
            }
        )

    image = Image.fromarray(
        np.clip(output, 0.0, 255.0).astype(np.uint8),
        mode="RGB",
    )
    output_directory.mkdir(parents=True, exist_ok=True)
    image_path = output_directory / "park_terrain_albedo_2560.png"
    image.save(image_path, optimize=True)
    preview_path = output_directory / "park_terrain_albedo_1024.png"
    image.resize((1024, 1024), Image.Resampling.LANCZOS).save(
        preview_path,
        optimize=True,
    )

    metadata = {
        "type": "terrain_albedo",
        "terrain": game_object.m_Name,
        "terrainData": terrain_data.m_Name,
        "terrainSize": [terrain_width, terrain_depth],
        "alphaResolution": alpha_resolution,
        "layers": layer_coverage,
        **bounds,
        "imageNorthAxis": "+Z",
        "image": image_path.name,
        "preview": preview_path.name,
    }
    (output_directory / "park_terrain_albedo_2560.json").write_text(
        json.dumps(metadata, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    return metadata


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Render the Park Terrain splat layers without scene lighting."
    )
    parser.add_argument(
        "--game-data",
        type=Path,
        required=True,
        help="Path to SecretFlasherManaka_Data/data.unity3d",
    )
    parser.add_argument(
        "--output-directory",
        type=Path,
        required=True,
    )
    parser.add_argument("--size", type=int, default=2560)
    args = parser.parse_args()
    metadata = render_terrain(
        args.game_data,
        args.output_directory,
        args.size,
        DEFAULT_BOUNDS,
    )
    print(json.dumps(metadata, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
