from __future__ import annotations

import argparse
import json
import math
import re
import time
from dataclasses import dataclass
from pathlib import Path

import numpy as np
import UnityPy
from PIL import Image, ImageDraw, ImageFont

from rasterize_park_models import (
    load_vertex_data,
    material_average,
    pointer_key,
    pointer_key_from_object,
    texture_colors,
    transform_points,
    world_transform_matrix,
)
from render_park_terrain_albedo import (
    find_park_terrain,
    sample_tiled,
)


LOD_SUFFIX = re.compile(r"_LOD[1-9](?:\D|$)")


SCENES = {
    "konbini": {
        "displayName": "便利店",
        "stageType": 6,
        "level": "level2",
        "rootName": "Environment",
        "includeBoxColliders": True,
        "floorHeights": [0.113],
    },
    "fashion_shop": {
        "displayName": "服装店",
        "stageType": 7,
        "level": "level6",
        "rootName": "Environment",
        "includeAllLods": True,
        "includeBoxColliders": True,
        "floorHeights": [0.075],
    },
    "residence": {
        "displayName": "住宅区",
        "stageType": 10,
        "level": "level8",
        "rootName": "Environment",
        "includeBoxColliders": True,
        "materialExposure": 1.20,
        "fillHollowBlockers": True,
        "floorHeights": [-0.10],
    },
    "station_front": {
        "displayName": "繁华街",
        "stageType": 12,
        "level": "level3",
        "rootName": "Environment",
        "includeBoxColliders": True,
        "floorHeights": [0.0],
    },
    "park": {
        "displayName": "综合公园",
        "stageType": 14,
        "level": "level5",
        "rootName": "Environment",
        "includeBoxColliders": True,
        "floorHeights": [49.002],
        "surfaceBelowMeters": 4.0,
        "surfaceUpperOffsetMeters": 4.0,
        "terrainLayer": True,
    },
    "mall": {
        "displayName": "购物中心",
        "stageType": 11,
        "level": "level4",
        "rootName": "Environment",
        "includeBoxColliders": True,
        "floorHeights": [-8.54, -3.15, 2.26],
    },
    "mansion": {
        "displayName": "公寓前",
        "stageType": 15,
        "level": "level7",
        "rootName": "Mansion_01 Variant",
        "includeBoxColliders": True,
        "includeAllGroundColliders": True,
        "fillHollowBlockers": True,
        "groundExtraRootNames": [
            "SM_building_residental_suburbs_02",
        ],
        "groundExpansionMeters": 45.0,
        "groundSurfaceCeilingMeters": 30.0,
        "floorHeights": [
            -0.01 + (22.70 + 0.01) * index / 9.0
            for index in range(10)
        ],
    },
}


@dataclass
class RenderStats:
    renderers: int = 0
    submeshes: int = 0
    floor_triangles: int = 0
    wall_triangles: int = 0
    floor_pixels: int = 0
    wall_pixels: int = 0
    collider_pixels: int = 0


def transform_path(
    transform,
    path_cache: dict,
    name_cache: dict,
    visiting: set[int] | None = None,
) -> tuple[str, ...]:
    key = pointer_key_from_object(transform)
    cached = path_cache.get(key)
    if cached is not None:
        return cached
    visiting = visiting or set()
    if key in visiting:
        return ()
    visiting.add(key)
    try:
        game_object = transform.m_GameObject.read()
        name = str(game_object.m_Name)
    except Exception:
        name = ""
    name_cache[pointer_key_from_object(game_object)] = name
    parent = transform.m_Father
    parent_parts: tuple[str, ...] = ()
    if parent is not None and parent.m_PathID:
        try:
            parent_parts = transform_path(
                parent.read(),
                path_cache,
                name_cache,
                visiting,
            )
        except Exception:
            parent_parts = ()
    visiting.discard(key)
    result = (*parent_parts, name)
    path_cache[key] = result
    return result


def renderer_record(
    renderer_reader,
    root_name: str,
    path_cache: dict,
    name_cache: dict,
    include_all_lods: bool = False,
) -> dict | None:
    try:
        renderer = renderer_reader.read()
        game_object = renderer.m_GameObject.read()
    except Exception:
        return None
    game_name = str(game_object.m_Name)
    if (
        not bool(renderer.m_Enabled)
        or not bool(game_object.m_IsActive)
        or int(game_object.m_Layer) not in (0, 20)
        or game_name.startswith("EFM_")
        or "Manustia" in game_name
        or "NPC" in game_name
        or (
            not include_all_lods
            and LOD_SUFFIX.search(game_name) is not None
        )
    ):
        return None

    transform = None
    mesh_filter = None
    for component_pair in game_object.m_Component:
        try:
            component = component_pair.component.read()
        except Exception:
            continue
        component_name = type(component).__name__
        if component_name == "Transform":
            transform = component
        elif component_name == "MeshFilter":
            mesh_filter = component
    if transform is None or mesh_filter is None:
        return None
    path = transform_path(transform, path_cache, name_cache)
    if root_name and root_name not in path:
        return None
    mesh_pointer = mesh_filter.m_Mesh
    if mesh_pointer.m_PathID == 0:
        return None
    return {
        "renderer": renderer,
        "gameObject": game_object,
        "gameName": game_name,
        "transform": transform,
        "meshPointer": mesh_pointer,
        "path": path,
    }


def box_collider_record(
    collider_reader,
    root_name: str,
    path_cache: dict,
    name_cache: dict,
) -> dict | None:
    try:
        collider = collider_reader.read()
        game_object = collider.m_GameObject.read()
    except Exception:
        return None
    game_name = str(game_object.m_Name)
    ignored = (
        "BOxFog",
        "Audio",
        "Light",
        "Trigger",
        "Area",
        "Camera",
    )
    if (
        not bool(game_object.m_IsActive)
        or not bool(collider.m_Enabled)
        or bool(collider.m_IsTrigger)
        or any(token.lower() in game_name.lower() for token in ignored)
    ):
        return None
    transform = None
    for component_pair in game_object.m_Component:
        try:
            component = component_pair.component.read()
        except Exception:
            continue
        if type(component).__name__ == "Transform":
            transform = component
            break
    if transform is None:
        return None
    path = transform_path(transform, path_cache, name_cache)
    if root_name and root_name not in path:
        return None
    center = collider.m_Center
    size = collider.m_Size
    if (
        float(size.x) <= 0.01
        or float(size.y) <= 0.01
        or float(size.z) <= 0.01
    ):
        return None
    return {
        "collider": collider,
        "gameObject": game_object,
        "gameName": game_name,
        "transform": transform,
        "center": np.array(
            [float(center.x), float(center.y), float(center.z)],
            dtype=np.float64,
        ),
        "size": np.array(
            [float(size.x), float(size.y), float(size.z)],
            dtype=np.float64,
        ),
    }


def selected_submeshes(mesh, renderer) -> tuple[list, int]:
    submeshes = list(mesh.m_SubMeshes)
    combined = str(mesh.m_Name).startswith("Combined Mesh (root: scene)")
    if combined and renderer.m_StaticBatchInfo is not None:
        first = int(renderer.m_StaticBatchInfo.firstSubMesh)
        count = int(renderer.m_StaticBatchInfo.subMeshCount)
        return submeshes[first : first + count], first
    return submeshes, 0


def mesh_triangles(world: np.ndarray, mesh, subset: list, offset: int):
    indices = np.frombuffer(
        bytes(mesh.m_IndexBuffer),
        dtype=np.uint16 if int(mesh.m_IndexFormat) == 0 else np.uint32,
    )
    item_size = np.dtype(indices.dtype).itemsize
    for local_index, submesh in enumerate(subset):
        start = int(submesh.firstByte) // item_size
        triangle_indices = indices[
            start : start + int(submesh.indexCount)
        ]
        if triangle_indices.size < 3:
            continue
        triangle_indices = triangle_indices[
            : triangle_indices.size - triangle_indices.size % 3
        ].reshape(-1, 3)
        if not triangle_indices.size:
            continue
        valid = triangle_indices.max(axis=1) < world.shape[0]
        triangle_indices = triangle_indices[valid]
        if not triangle_indices.size:
            continue
        triangles = world[triangle_indices].astype(
            np.float32,
            copy=False,
        )
        yield offset + local_index, triangles, triangle_indices


def triangle_normals(triangles: np.ndarray) -> tuple[np.ndarray, np.ndarray]:
    normals = np.cross(
        triangles[:, 1] - triangles[:, 0],
        triangles[:, 2] - triangles[:, 0],
    )
    lengths = np.linalg.norm(normals, axis=1)
    vertical_ratio = np.abs(normals[:, 1]) / np.maximum(
        lengths,
        1.0e-8,
    )
    return lengths, vertical_ratio


def project_triangle(
    triangle: np.ndarray,
    bounds: dict[str, float],
    size: int,
) -> tuple[float, float, float, float] | None:
    if (
        float(triangle[:, 0].max()) < bounds["minX"]
        or float(triangle[:, 0].min()) > bounds["maxX"]
        or float(triangle[:, 2].max()) < bounds["minZ"]
        or float(triangle[:, 2].min()) > bounds["maxZ"]
    ):
        return None
    source_x = (
        (triangle[:, 0] - bounds["minX"])
        / max(bounds["maxX"] - bounds["minX"], 1.0e-6)
        * (size - 1)
    )
    source_y = (
        (bounds["maxZ"] - triangle[:, 2])
        / max(bounds["maxZ"] - bounds["minZ"], 1.0e-6)
        * (size - 1)
    )
    return (
        max(0, int(math.floor(float(source_x.min())))),
        min(size - 1, int(math.ceil(float(source_x.max())))),
        max(0, int(math.floor(float(source_y.min())))),
        min(size - 1, int(math.ceil(float(source_y.max())))),
    )


def convex_hull(points: np.ndarray) -> np.ndarray:
    unique = sorted(
        {
            (round(float(point[0]), 7), round(float(point[1]), 7))
            for point in points
        }
    )
    if len(unique) <= 2:
        return np.asarray(unique, dtype=np.float64)

    def cross(
        origin: tuple[float, float],
        first: tuple[float, float],
        second: tuple[float, float],
    ) -> float:
        return (
            (first[0] - origin[0]) * (second[1] - origin[1])
            - (first[1] - origin[1]) * (second[0] - origin[0])
        )

    lower: list[tuple[float, float]] = []
    for point in unique:
        while (
            len(lower) >= 2
            and cross(lower[-2], lower[-1], point) <= 0.0
        ):
            lower.pop()
        lower.append(point)
    upper: list[tuple[float, float]] = []
    for point in reversed(unique):
        while (
            len(upper) >= 2
            and cross(upper[-2], upper[-1], point) <= 0.0
        ):
            upper.pop()
        upper.append(point)
    return np.asarray(
        lower[:-1] + upper[:-1],
        dtype=np.float64,
    )


def sample_terrain_values(
    values: np.ndarray,
    u: np.ndarray,
    v: np.ndarray,
) -> np.ndarray:
    height, width = values.shape
    x = np.clip(u, 0.0, 1.0) * (width - 1)
    y = np.clip(v, 0.0, 1.0) * (height - 1)
    x0 = np.floor(x).astype(np.int32)
    y0 = np.floor(y).astype(np.int32)
    x1 = np.minimum(x0 + 1, width - 1)
    y1 = np.minimum(y0 + 1, height - 1)
    fx = x - x0
    fy = y - y0
    top = values[y0, x0] * (1.0 - fx) + values[y0, x1] * fx
    bottom = values[y1, x0] * (1.0 - fx) + values[y1, x1] * fx
    return top * (1.0 - fy) + bottom * fy


def render_park_terrain_layer(
    environment: UnityPy.Environment,
    bounds: dict[str, float],
    size: int,
    material_exposure: float,
) -> tuple[np.ndarray, np.ndarray]:
    terrain, _game_object, terrain_data = find_park_terrain(environment)
    del terrain
    splat = terrain_data.m_SplatDatabase
    layers = [reference.read() for reference in splat.m_TerrainLayers]
    alpha_textures = [
        reference.read() for reference in splat.m_AlphaTextures
    ]
    if len(alpha_textures) != 2 or len(layers) not in (5, 8):
        raise RuntimeError(
            "Unsupported Park Terrain splat layout: "
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

    height_map = terrain_data.m_Heightmap
    height_scale = height_map.m_Scale
    resolution = int(height_map.m_Resolution)
    terrain_width = float(height_scale.x) * (resolution - 1)
    terrain_depth = float(height_scale.z) * (resolution - 1)
    raw_u = xx / max(terrain_width, 1.0e-6)
    raw_v = zz / max(terrain_depth, 1.0e-6)
    inside = (
        (raw_u >= 0.0)
        & (raw_u <= 1.0)
        & (raw_v >= 0.0)
        & (raw_v <= 1.0)
    )

    alpha0 = np.asarray(alpha_textures[0].image.convert("RGBA"))
    alpha1 = np.asarray(alpha_textures[1].image.convert("RGBA"))
    alpha_size = alpha0.shape[0]
    alpha_x = np.clip(
        (raw_u * (alpha_size - 1)).astype(np.int32),
        0,
        alpha_size - 1,
    )
    alpha_y = np.clip(
        (raw_v * (alpha_size - 1)).astype(np.int32),
        0,
        alpha_size - 1,
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

    albedo = np.zeros((size, size, 3), dtype=np.float32)
    for index, layer in enumerate(layers):
        texture = layer.m_DiffuseTexture.read()
        diffuse = np.asarray(texture.image.convert("RGB"))
        tile_size = layer.m_TileSize
        tile_offset = layer.m_TileOffset
        u = xx / max(abs(float(tile_size.x)), 1.0e-6) - float(
            tile_offset.x
        )
        v = zz / max(abs(float(tile_size.y)), 1.0e-6) - float(
            tile_offset.y
        )
        albedo += sample_tiled(diffuse, u, v) * weights[
            ..., index
        ][..., None]

    albedo_colors = map_material_color(
        albedo / 255.0,
        material_exposure,
    )
    raw_heights = np.asarray(
        height_map.m_Heights,
        dtype=np.float32,
    ).reshape(resolution, resolution)
    heights = sample_terrain_values(raw_heights, raw_u, raw_v)
    sampled = heights[inside]
    if sampled.size:
        low, high = np.percentile(sampled, [1.0, 99.0])
    else:
        low, high = float(heights.min()), float(heights.max())
    level = np.clip(
        (heights - low) / max(float(high - low), 1.0),
        0.0,
        1.0,
    )

    smooth = heights.copy()
    for _ in range(4):
        smooth = (
            smooth
            + np.roll(smooth, 1, axis=0)
            + np.roll(smooth, -1, axis=0)
            + np.roll(smooth, 1, axis=1)
            + np.roll(smooth, -1, axis=1)
        ) / 5.0
    gradient_y, gradient_x = np.gradient(smooth)
    normals = np.dstack(
        (
            -gradient_x * 18.0,
            np.ones_like(smooth),
            -gradient_y * 18.0,
        )
    )
    normals /= np.maximum(
        np.linalg.norm(normals, axis=2, keepdims=True),
        1.0e-6,
    )
    light = np.asarray([-0.45, 0.82, -0.35], dtype=np.float32)
    light /= np.linalg.norm(light)
    shade = 0.78 + 0.28 * np.clip(normals @ light, 0.0, 1.0)
    low_green = np.asarray([22, 57, 31], dtype=np.float32) / 255.0
    high_green = np.asarray([145, 190, 108], dtype=np.float32) / 255.0
    green_gradient = (
        low_green + (high_green - low_green) * level[..., None]
    )
    albedo_luminance = (
        albedo_colors[..., 0] * 0.2126
        + albedo_colors[..., 1] * 0.7152
        + albedo_colors[..., 2] * 0.0722
    )
    texture_detail = np.clip(
        0.84 + albedo_luminance * 0.28,
        0.84,
        1.08,
    )
    colors = np.clip(
        green_gradient * shade[..., None] * texture_detail[..., None],
        0.0,
        1.0,
    )
    return (
        (colors * 255.0).astype(np.uint8),
        inside.astype(np.uint8),
    )


def rasterize_floor_triangle(
    triangle: np.ndarray,
    triangle_uv: np.ndarray,
    base_color: np.ndarray,
    texture: np.ndarray | None,
    uv_scale: np.ndarray,
    floor_y: float,
    bounds: dict[str, float],
    output: np.ndarray,
    valid: np.ndarray,
    visible_heights: np.ndarray,
    material_exposure: float,
    surface_below_offset: float,
    surface_upper_offset: float,
) -> int:
    size = output.shape[0]
    projected = project_triangle(triangle, bounds, size)
    if projected is None:
        return 0
    x0, x1, y0, y1 = projected
    source_x = (
        (triangle[:, 0] - bounds["minX"])
        / max(bounds["maxX"] - bounds["minX"], 1.0e-6)
        * (size - 1)
    )
    source_y = (
        (bounds["maxZ"] - triangle[:, 2])
        / max(bounds["maxZ"] - bounds["minZ"], 1.0e-6)
        * (size - 1)
    )
    area = (
        (float(source_x[1]) - float(source_x[0]))
        * (float(source_y[2]) - float(source_y[0]))
        - (float(source_x[2]) - float(source_x[0]))
        * (float(source_y[1]) - float(source_y[0]))
    )
    if abs(area) <= 1.0e-8:
        return 0
    yy, xx = np.mgrid[y0 : y1 + 1, x0 : x1 + 1]
    weight_a = (
        (source_x[1] - xx) * (source_y[2] - yy)
        - (source_x[2] - xx) * (source_y[1] - yy)
    ) / area
    weight_b = (
        (source_x[2] - xx) * (source_y[0] - yy)
        - (source_x[0] - xx) * (source_y[2] - yy)
    ) / area
    weight_c = 1.0 - weight_a - weight_b
    inside = (
        (weight_a >= -1.0e-5)
        & (weight_b >= -1.0e-5)
        & (weight_c >= -1.0e-5)
    )
    if not inside.any():
        return 0
    surface_y = (
        triangle[0, 1] * weight_a
        + triangle[1, 1] * weight_b
        + triangle[2, 1] * weight_c
    )
    patch_heights = visible_heights[
        y0 : y1 + 1,
        x0 : x1 + 1,
    ]
    candidate = (
        inside
        & (surface_y >= floor_y - surface_below_offset)
        & (surface_y <= floor_y + surface_upper_offset)
    )
    accepted = candidate & (
        surface_y > patch_heights + 1.0e-5
    )
    count = int(accepted.sum())
    if count == 0:
        return 0
    patch_heights[accepted] = surface_y[accepted]
    absolute_y = y0 + np.nonzero(accepted)[0]
    absolute_x = x0 + np.nonzero(accepted)[1]
    accepted_uv = (
        triangle_uv[0, :][None, :] * weight_a[accepted, None]
        + triangle_uv[1, :][None, :] * weight_b[accepted, None]
        + triangle_uv[2, :][None, :] * weight_c[accepted, None]
    )
    colors = map_material_color(
        texture_colors(
            texture,
            uv_scale,
            accepted_uv,
            base_color,
        ),
        material_exposure,
    )
    output[absolute_y, absolute_x] = (
        np.clip(colors, 0.0, 1.0) * 255.0
    ).astype(np.uint8)
    valid[absolute_y, absolute_x] = 1
    return count


def rasterize_wall_triangle(
    triangle: np.ndarray,
    bounds: dict[str, float],
    wall_mask: np.ndarray,
) -> int:
    size = wall_mask.shape[0]
    projected = project_triangle(triangle, bounds, size)
    if projected is None:
        return 0
    x0, x1, y0, y1 = projected
    source_x = (
        (triangle[:, 0] - bounds["minX"])
        / max(bounds["maxX"] - bounds["minX"], 1.0e-6)
        * (size - 1)
    )
    source_y = (
        (bounds["maxZ"] - triangle[:, 2])
        / max(bounds["maxZ"] - bounds["minZ"], 1.0e-6)
        * (size - 1)
    )
    area = (
        (float(source_x[1]) - float(source_x[0]))
        * (float(source_y[2]) - float(source_y[0]))
        - (float(source_x[2]) - float(source_x[0]))
        * (float(source_y[1]) - float(source_y[0]))
    )
    if abs(area) <= 1.0e-8:
        return 0
    yy, xx = np.mgrid[y0 : y1 + 1, x0 : x1 + 1]
    weight_a = (
        (source_x[1] - xx) * (source_y[2] - yy)
        - (source_x[2] - xx) * (source_y[1] - yy)
    ) / area
    weight_b = (
        (source_x[2] - xx) * (source_y[0] - yy)
        - (source_x[0] - xx) * (source_y[2] - yy)
    ) / area
    weight_c = 1.0 - weight_a - weight_b
    inside = (
        (weight_a >= -1.0e-5)
        & (weight_b >= -1.0e-5)
        & (weight_c >= -1.0e-5)
    )
    if not inside.any():
        return 0
    patch = wall_mask[y0 : y1 + 1, x0 : x1 + 1]
    before = int(patch.sum())
    patch[inside] = 1
    return int(patch.sum()) - before


def rasterize_box_collider(
    record: dict,
    floor_y: float,
    bounds: dict[str, float],
    blocker_mask: np.ndarray,
    pillar_mask: np.ndarray,
    ground_building_mask: np.ndarray,
    transform_cache: dict,
) -> int:
    matrix = world_transform_matrix(
        record["transform"],
        transform_cache,
        set(),
    )
    half = record["size"] * 0.5
    corners = np.array(
        [
            [
                record["center"][0] + x * half[0],
                record["center"][1] + y * half[1],
                record["center"][2] + z * half[2],
            ]
            for x in (-1.0, 1.0)
            for y in (-1.0, 1.0)
            for z in (-1.0, 1.0)
        ],
        dtype=np.float64,
    )
    world = transform_points(corners, matrix)
    y_min = float(world[:, 1].min())
    y_max = float(world[:, 1].max())
    if y_max <= floor_y + 0.20 or y_min >= floor_y + 1.55:
        return 0
    footprint = convex_hull(world[:, [0, 2]])
    if footprint.shape[0] < 3:
        return 0
    size = blocker_mask.shape[0]
    source_x = (
        (footprint[:, 0] - bounds["minX"])
        / max(bounds["maxX"] - bounds["minX"], 1.0e-6)
        * (size - 1)
    )
    source_y = (
        (bounds["maxZ"] - footprint[:, 1])
        / max(bounds["maxZ"] - bounds["minZ"], 1.0e-6)
        * (size - 1)
    )
    x0 = max(
        0,
        int(
            math.floor(float(source_x.min()))
        ),
    )
    x1 = min(
        size - 1,
        int(
            math.ceil(float(source_x.max()))
        ),
    )
    y0 = max(
        0,
        int(
            math.floor(float(source_y.min()))
        ),
    )
    y1 = min(
        size - 1,
        int(
            math.ceil(float(source_y.max()))
        ),
    )
    if x1 < x0 or y1 < y0:
        return 0
    yy, xx = np.mgrid[y0 : y1 + 1, x0 : x1 + 1]
    inside = np.ones_like(xx, dtype=bool)
    signed_area = 0.5 * float(
        np.dot(source_x, np.roll(source_y, -1))
        - np.dot(source_y, np.roll(source_x, -1))
    )
    for index in range(footprint.shape[0]):
        next_index = (index + 1) % footprint.shape[0]
        cross = (
            (source_x[next_index] - source_x[index]) *
                (yy - source_y[index])
            - (source_y[next_index] - source_y[index]) *
                (xx - source_x[index])
        )
        inside &= (
            cross >= -1.0e-5
            if signed_area >= 0.0
            else cross <= 1.0e-5
        )
    patch = blocker_mask[y0 : y1 + 1, x0 : x1 + 1]
    before = int(patch.sum())
    patch[inside] = 1
    if "pillar" in record["gameName"].lower():
        pillar_mask[
            y0 : y1 + 1,
            x0 : x1 + 1,
        ][inside] = 1
    if record.get("groundOnly", False):
        ground_building_mask[
            y0 : y1 + 1,
            x0 : x1 + 1,
        ][inside] = 1
    return int(patch.sum()) - before


def map_material_color(
    sample: np.ndarray,
    exposure: float = 1.0,
) -> np.ndarray:
    sample = np.clip(sample, 0.0, 1.0)
    luminance = (
        sample[..., 0] * 0.2126
        + sample[..., 1] * 0.7152
        + sample[..., 2] * 0.0722
    )
    level = np.clip(
        (luminance - 0.56) / 0.34,
        0.0,
        1.0,
    )
    low = np.array([0.46, 0.41, 0.33], dtype=np.float32)
    high = np.array([0.82, 0.74, 0.61], dtype=np.float32)
    lifted = low + (high - low) * level[..., None]
    lifted += (
        sample - luminance[..., None]
    ) * 0.40
    lifted = lifted * 0.84 + sample * 0.16
    lifted = lifted + np.array(
        [0.70, 0.66, 0.58],
        dtype=np.float32,
    ) * (luminance[..., None] - 0.5) * 0.035
    return np.clip(lifted * max(0.0, exposure), 0.0, 1.0)


def edge_mask(mask: np.ndarray) -> np.ndarray:
    edge = np.zeros_like(mask, dtype=bool)
    edge[1:, :] |= mask[1:, :] != mask[:-1, :]
    edge[:-1, :] |= mask[:-1, :] != mask[1:, :]
    edge[:, 1:] |= mask[:, 1:] != mask[:, :-1]
    edge[:, :-1] |= mask[:, :-1] != mask[:, 1:]
    return edge


def dilate_binary(mask: np.ndarray, radius: int) -> np.ndarray:
    result = mask.astype(bool, copy=True)
    for _ in range(max(0, radius)):
        previous = result.copy()
        result[1:, :] |= previous[:-1, :]
        result[:-1, :] |= previous[1:, :]
        result[:, 1:] |= previous[:, :-1]
        result[:, :-1] |= previous[:, 1:]
    return result


def erode_binary(mask: np.ndarray, radius: int) -> np.ndarray:
    result = mask.astype(bool, copy=True)
    for _ in range(max(0, radius)):
        previous = result.copy()
        result[1:, :] &= previous[:-1, :]
        result[:-1, :] &= previous[1:, :]
        result[:, 1:] &= previous[:, :-1]
        result[:, :-1] &= previous[:, 1:]
        result[0, :] = False
        result[-1, :] = False
        result[:, 0] = False
        result[:, -1] = False
    return result


def fill_mask_holes(mask: np.ndarray) -> np.ndarray:
    height, width = mask.shape
    padded = Image.new("L", (width + 2, height + 2), 0)
    padded.paste(
        Image.fromarray(
            np.where(mask, 255, 0).astype(np.uint8),
            mode="L",
        ),
        (1, 1),
    )
    ImageDraw.floodfill(
        padded,
        (0, 0),
        128,
        thresh=0,
    )
    enclosed = (
        np.asarray(padded, dtype=np.uint8)[1:-1, 1:-1] == 0
    )
    return mask.astype(bool, copy=False) | enclosed


def finish_floor(
    output: np.ndarray,
    valid: np.ndarray,
    wall_mask: np.ndarray,
    blocker_mask: np.ndarray,
    pillar_mask: np.ndarray,
    ground_building_mask: np.ndarray,
    fill_hollow_blockers: bool,
) -> np.ndarray:
    if output.ndim != 3 or output.shape[2] != 3:
        raise RuntimeError(f"Unexpected floor output shape: {output.shape}")
    if valid.shape != output.shape[:2]:
        raise RuntimeError(
            f"Invalid floor mask shape: {valid.shape} vs {output.shape[:2]}"
        )
    if wall_mask.shape != output.shape[:2]:
        raise RuntimeError(
            f"Invalid wall mask shape: {wall_mask.shape} vs {output.shape[:2]}"
        )
    if blocker_mask.shape != output.shape[:2]:
        raise RuntimeError(
            "Invalid blocker mask shape: "
            f"{blocker_mask.shape} vs {output.shape[:2]}"
        )
    if pillar_mask.shape != output.shape[:2]:
        raise RuntimeError(
            "Invalid pillar mask shape: "
            f"{pillar_mask.shape} vs {output.shape[:2]}"
        )
    if ground_building_mask.shape != output.shape[:2]:
        raise RuntimeError(
            "Invalid ground building mask shape: "
            f"{ground_building_mask.shape} vs {output.shape[:2]}"
        )
    wall_mask = wall_mask.astype(bool, copy=False)
    blocker_mask = blocker_mask.astype(bool, copy=False)
    pillar_mask = pillar_mask.astype(bool, copy=False)
    ground_building_mask = ground_building_mask.astype(
        bool,
        copy=False,
    )
    background = np.array([28, 39, 41], dtype=np.uint8)
    result = np.where(valid[..., None], output, background).astype(np.uint8)
    result[wall_mask] = (
        result[wall_mask].astype(np.float32) * 0.35
        + np.array([24.0, 32.0, 33.0]) * 0.65
    ).astype(np.uint8)
    borders = edge_mask(valid)
    result[borders] = (
        result[borders].astype(np.float32) * 0.55
        + np.array([220.0, 209.0, 181.0]) * 0.45
    ).astype(np.uint8)
    result[blocker_mask] = (
        result[blocker_mask].astype(np.float32) * 0.82
        + np.array([87.0, 82.0, 70.0]) * 0.18
    ).astype(np.uint8)
    blocker_luminance = (
        result[..., 0].astype(np.float32) * 0.2126
        + result[..., 1].astype(np.float32) * 0.7152
        + result[..., 2].astype(np.float32) * 0.0722
    )
    filled_blockers = (
        erode_binary(
            fill_mask_holes(dilate_binary(blocker_mask, 8)),
            8,
        )
        if fill_hollow_blockers
        else blocker_mask
    )
    hollow_blockers = filled_blockers & (blocker_luminance < 72.0)
    result[hollow_blockers] = (
        result[hollow_blockers].astype(np.float32) * 0.18
        + np.array([126.0, 108.0, 81.0]) * 0.82
    ).astype(np.uint8)
    filled_ground_buildings = (
        erode_binary(
            fill_mask_holes(dilate_binary(ground_building_mask, 8)),
            8,
        )
        if fill_hollow_blockers
        else ground_building_mask
    )
    dark_ground_buildings = filled_ground_buildings & (
        blocker_luminance < 82.0
    )
    result[dark_ground_buildings] = (
        result[dark_ground_buildings].astype(np.float32) * 0.14
        + np.array([133.0, 115.0, 87.0]) * 0.86
    ).astype(np.uint8)
    blocker_edges = dilate_binary(edge_mask(blocker_mask), 1)
    result[blocker_edges] = (
        result[blocker_edges].astype(np.float32) * 0.35
        + np.array([207.0, 184.0, 139.0]) * 0.65
    ).astype(np.uint8)
    result[pillar_mask] = (
        result[pillar_mask].astype(np.float32) * 0.46
        + np.array([92.0, 83.0, 64.0]) * 0.54
    ).astype(np.uint8)
    pillar_edges = dilate_binary(edge_mask(pillar_mask), 2)
    result[pillar_edges] = (
        result[pillar_edges].astype(np.float32) * 0.12
        + np.array([242.0, 207.0, 126.0]) * 0.88
    ).astype(np.uint8)
    return result


def make_contact_sheet(
    images: list[tuple[str, Image.Image]],
    output_path: Path,
    sheet_size: int = 1024,
) -> None:
    columns = 5 if len(images) > 3 else 3
    rows = math.ceil(len(images) / columns)
    gap = 12
    label_height = 30
    tile = (
        sheet_size - gap * (columns + 1)
    ) // columns
    cell_height = tile + label_height
    height = gap + rows * (cell_height + gap)
    sheet = Image.new("RGB", (sheet_size, height), (17, 23, 24))
    draw = ImageDraw.Draw(sheet)
    font = ImageFont.load_default()
    for index, (label, image) in enumerate(images):
        row, column = divmod(index, columns)
        x = gap + column * (tile + gap)
        y = gap + row * (cell_height + gap)
        draw.text((x + 4, y + 7), label, fill=(224, 218, 198), font=font)
        preview = image.resize((tile, tile), Image.Resampling.LANCZOS)
        sheet.paste(preview, (x, y + label_height))
    sheet.save(output_path, optimize=True)


def compute_bounds(
    renderer_records: list[dict],
    mesh_cache: dict,
    transform_cache: dict,
    bundle_files: dict,
    floor_heights: list[float],
) -> tuple[dict[str, float], RenderStats]:
    samples: list[np.ndarray] = []
    stats = RenderStats()
    lowest = min(floor_heights) - 0.8
    highest = max(floor_heights) + 0.9

    for record in renderer_records:
        mesh_pointer = record["meshPointer"]
        key = pointer_key(mesh_pointer)
        if key not in mesh_cache:
            try:
                mesh = mesh_pointer.read()
                mesh_cache[key] = (
                    mesh,
                    *load_vertex_data(mesh, bundle_files),
                )
            except Exception:
                mesh_cache[key] = (None, None, None, None)
        mesh, positions, _uvs, _indices = mesh_cache[key]
        if mesh is None or positions is None:
            continue
        combined = str(mesh.m_Name).startswith("Combined Mesh (root: scene)")
        if combined:
            world = positions.astype(np.float32, copy=False)
        else:
            matrix = world_transform_matrix(
                record["transform"],
                transform_cache,
                set(),
            )
            world = transform_points(positions, matrix).astype(np.float32)
        relevant = world[
            (world[:, 1] >= lowest)
            & (world[:, 1] <= highest)
            & np.isfinite(world).all(axis=1)
        ]
        if not relevant.size:
            continue
        stats.renderers += 1
        stride = max(1, relevant.shape[0] // 4096)
        samples.append(relevant[::stride, [0, 2]])

    coordinates = np.concatenate(samples, axis=0)
    low = np.percentile(coordinates, 0.25, axis=0)
    high = np.percentile(coordinates, 99.75, axis=0)
    min_x, min_z = (float(value) for value in low)
    max_x, max_z = (float(value) for value in high)
    margin_x = max((max_x - min_x) * 0.012, 0.4)
    margin_z = max((max_z - min_z) * 0.012, 0.4)
    return {
        "minX": min_x - margin_x,
        "maxX": max_x + margin_x,
        "minY": min(floor_heights),
        "maxY": max(floor_heights),
        "minZ": min_z - margin_z,
        "maxZ": max_z + margin_z,
    }, stats


def render_floor(
    renderer_records: list[dict],
    mesh_cache: dict,
    transform_cache: dict,
    material_cache: dict,
    bundle_files: dict,
    collider_records: list[dict],
    floor_y: float,
    bounds: dict[str, float],
    size: int,
    material_exposure: float,
    ground_surface_ceiling: float,
    fill_hollow_blockers: bool,
    surface_below_offset: float,
    surface_upper_offset: float,
    terrain_layer: tuple[np.ndarray, np.ndarray] | None,
) -> tuple[np.ndarray, np.ndarray, np.ndarray, RenderStats]:
    output = np.zeros((size, size, 3), dtype=np.uint8)
    valid = np.zeros((size, size), dtype=np.uint8)
    wall_mask = np.zeros((size, size), dtype=np.uint8)
    blocker_mask = np.zeros((size, size), dtype=np.uint8)
    pillar_mask = np.zeros((size, size), dtype=np.uint8)
    ground_building_mask = np.zeros((size, size), dtype=np.uint8)
    visible_heights = np.full(
        (size, size),
        -1.0e20,
        dtype=np.float32,
    )
    stats = RenderStats()

    for record in renderer_records:
        mesh_pointer = record["meshPointer"]
        key = pointer_key(mesh_pointer)
        mesh, positions, uvs, _indices = mesh_cache[key]
        if mesh is None or positions is None:
            continue
        combined = str(mesh.m_Name).startswith("Combined Mesh (root: scene)")
        if combined:
            world = positions.astype(np.float32, copy=False)
        else:
            matrix = world_transform_matrix(
                record["transform"],
                transform_cache,
                set(),
            )
            world = transform_points(positions, matrix).astype(np.float32)

        subset, offset = selected_submeshes(mesh, record["renderer"])
        if not subset:
            continue
        material_list = []
        for material_pointer in record["renderer"].m_Materials:
            material_key = pointer_key(material_pointer)
            if material_key not in material_cache:
                try:
                    material_cache[material_key] = material_average(
                        material_pointer.read()
                    )
                except Exception:
                    material_cache[material_key] = (
                        np.ones(3, dtype=np.float32),
                        None,
                        np.ones(2, dtype=np.float32),
                    )
            material_list.append(material_cache[material_key])
        if not material_list:
            material_list = [
                (
                    np.ones(3, dtype=np.float32),
                    None,
                    np.ones(2, dtype=np.float32),
                )
            ]

        stats.renderers += 1
        record_surface_upper_offset = (
            ground_surface_ceiling
            if record.get("groundOnly", False)
            else surface_upper_offset
        )
        for local_index, triangles, triangle_indices in mesh_triangles(
            world,
            mesh,
            subset,
            0,
        ):
            stats.submeshes += 1
            lengths, upward_ratio = triangle_normals(triangles)
            y_min = triangles[:, :, 1].min(axis=1)
            y_max = triangles[:, :, 1].max(axis=1)

            upward = (
                (lengths > 1.0e-8)
                & (upward_ratio >= 0.35)
                & (y_max >= floor_y - surface_below_offset)
                & (y_min <= floor_y + record_surface_upper_offset)
            )
            vertical = (
                (lengths > 1.0e-8)
                & (upward_ratio < 0.35)
                & (y_max >= floor_y + 0.05)
                & (y_min <= floor_y + 2.15)
            )
            if not upward.any() and not vertical.any():
                continue

            material_index = min(local_index, len(material_list) - 1)
            base_color, texture, uv_scale = material_list[material_index]

            for triangle_index in np.nonzero(upward)[0]:
                triangle = triangles[triangle_index]
                if uvs is None:
                    triangle_uv = np.zeros((3, 2), dtype=np.float32)
                else:
                    triangle_uv = uvs[
                        triangle_indices[triangle_index]
                    ]
                stats.floor_pixels += rasterize_floor_triangle(
                    triangle,
                    triangle_uv,
                    base_color,
                    texture,
                    uv_scale,
                    floor_y,
                    bounds,
                    output,
                    valid,
                    visible_heights,
                    material_exposure,
                    surface_below_offset,
                    record_surface_upper_offset,
                )
                stats.floor_triangles += 1

            for triangle_index in np.nonzero(vertical)[0]:
                stats.wall_pixels += rasterize_wall_triangle(
                    triangles[triangle_index],
                    bounds,
                    wall_mask,
                )
                stats.wall_triangles += 1
        if stats.renderers % 250 == 0:
            print(
                f"floor={floor_y:.3f} renderers={stats.renderers} "
                f"floorTriangles={stats.floor_triangles} "
                f"wallTriangles={stats.wall_triangles} "
                f"floorPixels={stats.floor_pixels}",
                flush=True,
            )
    for collider_record in collider_records:
        stats.collider_pixels += rasterize_box_collider(
            collider_record,
            floor_y,
            bounds,
            blocker_mask,
            pillar_mask,
            ground_building_mask,
            transform_cache,
        )
    if terrain_layer is not None:
        terrain_colors, terrain_valid = terrain_layer
        terrain_mask = terrain_valid.astype(bool, copy=False)
        fill = terrain_mask & ~valid.astype(bool, copy=False)
        output[fill] = terrain_colors[fill]
        valid[terrain_mask] = 1
    return (
        finish_floor(
            output,
            valid,
            wall_mask,
            blocker_mask,
            pillar_mask,
            ground_building_mask,
            fill_hollow_blockers,
        ),
        valid,
        wall_mask,
        blocker_mask,
        stats,
    )


def build_scene(
    scene_key: str,
    game_data: Path,
    output_root: Path,
    size: int,
    preview_size: int,
    debug_only: bool,
) -> dict:
    config = SCENES[scene_key]
    floor_heights = [round(float(value), 5) for value in config["floorHeights"]]
    print(
        f"loading {scene_key}: level={config['level']} "
        f"floors={floor_heights}",
        flush=True,
    )
    start = time.time()
    environment = UnityPy.load(str(game_data))
    environment_objects = list(environment.objects)
    bundle = next(iter(environment.files.values()))
    level_objects = [
        obj
        for obj in environment_objects
        if getattr(getattr(obj, "assets_file", None), "name", None)
        == config["level"]
    ]
    path_cache: dict = {}
    name_cache: dict = {}
    renderer_records = []
    collider_records = []
    extra_ground_root_names = list(
        config.get("groundExtraRootNames", [])
    )
    for reader in level_objects:
        if reader.type.name == "MeshRenderer":
            record = renderer_record(
                reader,
                config["rootName"],
                path_cache,
                name_cache,
                bool(config.get("includeAllLods", False)),
            )
            if record is not None:
                renderer_records.append(record)
            else:
                for extra_root_name in extra_ground_root_names:
                    record = renderer_record(
                        reader,
                        extra_root_name,
                        path_cache,
                        name_cache,
                        bool(config.get("includeAllLods", False)),
                    )
                    if record is not None:
                        record["groundOnly"] = True
                        renderer_records.append(record)
                        break
        if (
            config.get("includeBoxColliders", False)
            and reader.type.name == "BoxCollider"
        ):
            collider_record = box_collider_record(
                reader,
                config["rootName"],
                path_cache,
                name_cache,
            )
            if collider_record is not None:
                collider_record["groundOnly"] = False
            else:
                for extra_root_name in extra_ground_root_names:
                    collider_record = box_collider_record(
                        reader,
                        extra_root_name,
                        path_cache,
                        name_cache,
                    )
                    if collider_record is not None:
                        collider_record["groundOnly"] = True
                        break
            if (
                collider_record is None
                and config.get("includeAllGroundColliders", False)
            ):
                collider_record = box_collider_record(
                    reader,
                    "",
                    path_cache,
                    name_cache,
                )
                if collider_record is not None:
                    collider_record["groundOnly"] = True
            if collider_record is not None:
                name_filters = config.get("colliderNameIncludes", [])
                if name_filters and not any(
                    token.lower() in collider_record["gameName"].lower()
                    for token in name_filters
                ):
                    continue
                collider_records.append(collider_record)
    print(
        f"{scene_key}: selected renderers={len(renderer_records)} "
        f"load={time.time() - start:.1f}s",
        flush=True,
    )

    mesh_cache: dict = {}
    transform_cache: dict = {}
    material_cache: dict = {}
    primary_records = [
        record
        for record in renderer_records
        if not record.get("groundOnly", False)
    ]
    ground_records = [
        record
        for record in renderer_records
        if record.get("groundOnly", False)
    ]
    primary_bounds, _ = compute_bounds(
        primary_records,
        mesh_cache,
        transform_cache,
        bundle.files,
        floor_heights,
    )
    bounds = primary_bounds
    if ground_records:
        ground_bounds, _ = compute_bounds(
            ground_records,
            mesh_cache,
            transform_cache,
            bundle.files,
            floor_heights,
        )
        expansion = max(
            0.0,
            float(config.get("groundExpansionMeters", 0.0)),
        )
        ground_min_x = max(
            primary_bounds["minX"] - expansion,
            ground_bounds["minX"],
        )
        ground_max_x = min(
            primary_bounds["maxX"] + expansion,
            ground_bounds["maxX"],
        )
        ground_min_z = max(
            primary_bounds["minZ"] - expansion,
            ground_bounds["minZ"],
        )
        ground_max_z = min(
            primary_bounds["maxZ"] + expansion,
            ground_bounds["maxZ"],
        )
        bounds = {
            "minX": min(primary_bounds["minX"], ground_min_x),
            "maxX": max(primary_bounds["maxX"], ground_max_x),
            "minY": min(primary_bounds["minY"], ground_bounds["minY"]),
            "maxY": max(primary_bounds["maxY"], ground_bounds["maxY"]),
            "minZ": min(primary_bounds["minZ"], ground_min_z),
            "maxZ": max(primary_bounds["maxZ"], ground_max_z),
        }
    print(
        f"{scene_key}: bounds={json.dumps(bounds)}",
        flush=True,
    )
    material_exposure = float(config.get("materialExposure", 1.0))
    terrain_layer = (
        render_park_terrain_layer(
            environment,
            bounds,
            size,
            material_exposure,
        )
        if config.get("terrainLayer", False)
        else None
    )

    scene_directory = output_root / scene_key
    scene_directory.mkdir(parents=True, exist_ok=True)
    floor_metadata = []
    thumbnails: list[tuple[str, Image.Image]] = []
    for index, floor_y in enumerate(floor_heights, 1):
        floor_start = time.time()
        include_ground = index == 1 and bool(ground_records)
        floor_records = (
            renderer_records if include_ground else primary_records
        )
        floor_colliders = [
            record
            for record in collider_records
            if include_ground or not record.get("groundOnly", False)
        ]
        floor_bounds = bounds if include_ground else primary_bounds
        image, valid, wall_mask, blocker_mask, stats = render_floor(
            floor_records,
            mesh_cache,
            transform_cache,
            material_cache,
            bundle.files,
            floor_colliders,
            floor_y,
            floor_bounds,
            size,
            material_exposure,
            float(config.get("groundSurfaceCeilingMeters", 30.0)),
            bool(config.get("fillHollowBlockers", False)),
            float(config.get("surfaceBelowMeters", 0.55)),
            float(config.get("surfaceUpperOffsetMeters", 1.55)),
            terrain_layer,
        )
        if debug_only:
            actual_path = scene_directory / f"floor_{index:02d}_512_debug.png"
            preview = Image.fromarray(image, mode="RGB")
            preview.save(actual_path, optimize=True)
        else:
            actual_path = scene_directory / f"floor_{index:02d}_2560.png"
            Image.fromarray(image, mode="RGB").save(
                actual_path,
                optimize=True,
            )
            preview_path = scene_directory / f"floor_{index:02d}_512.png"
            preview = Image.fromarray(image, mode="RGB").resize(
                (preview_size, preview_size),
                Image.Resampling.LANCZOS,
            )
            preview.save(preview_path, optimize=True)
            valid.astype(np.uint8).tofile(
                scene_directory / f"floor_{index:02d}_valid_2560.r8"
            )
            wall_mask.astype(np.uint8).tofile(
                scene_directory / f"floor_{index:02d}_walls_2560.r8"
            )
        thumbnails.append((f"{index}F  y={floor_y:.2f}", preview))
        floor_metadata.append(
            {
                "index": index,
                "label": f"{index}F",
                "height": floor_y,
                "image2560": (
                    actual_path.name if not debug_only else None
                ),
                "preview512": (
                    preview_path.name if not debug_only else actual_path.name
                ),
                **floor_bounds,
                "renderers": stats.renderers,
                "floorTriangles": stats.floor_triangles,
                "wallTriangles": stats.wall_triangles,
                "floorPixels": stats.floor_pixels,
                "wallPixels": stats.wall_pixels,
                "colliderPixels": stats.collider_pixels,
            }
        )
        print(
            f"{scene_key} {index}F height={floor_y:.3f} "
            f"triangles={stats.floor_triangles}+{stats.wall_triangles} "
            f"pixels={stats.floor_pixels}/{stats.wall_pixels} "
            f"time={time.time() - floor_start:.1f}s",
            flush=True,
        )

    contact_name = f"all_floors_{'512_debug' if debug_only else '512'}.png"
    contact_path = scene_directory / contact_name
    make_contact_sheet(thumbnails, contact_path)
    metadata = {
        "schemaVersion": 1,
        "scene": scene_key,
        "displayName": config["displayName"],
        "stageType": config["stageType"],
        "level": config["level"],
        "rootName": config["rootName"],
        "extraGroundRootNames": extra_ground_root_names,
        "debugOnly": debug_only,
        "imageSize": size,
        "previewSize": preview_size,
        "bounds": bounds,
        "floorCount": len(floor_metadata),
        "floors": floor_metadata,
        "contactSheet": contact_path.name,
    }
    (scene_directory / "manifest.json").write_text(
        json.dumps(metadata, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    return metadata


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Render per-floor maps for Mall and Mansion."
    )
    parser.add_argument("--game-data", type=Path, required=True)
    parser.add_argument("--output-directory", type=Path, required=True)
    parser.add_argument(
        "--scene",
        action="append",
        choices=sorted(SCENES),
        default=[],
    )
    parser.add_argument("--size", type=int, default=2560)
    parser.add_argument("--preview-size", type=int, default=512)
    parser.add_argument("--debug-512", action="store_true")
    parser.add_argument(
        "--write-manifest-only",
        action="store_true",
        help="Only rebuild atlas_manifest.json from existing scene manifests.",
    )
    args = parser.parse_args()
    scenes = [] if args.write_manifest_only else (args.scene or list(SCENES))
    results = []
    for scene_key in scenes:
        results.append(
            build_scene(
                scene_key,
                args.game_data,
                args.output_directory,
                args.size if not args.debug_512 else args.preview_size,
                args.preview_size,
                args.debug_512,
            )
        )
    atlas_scenes = []
    for scene_key, config in SCENES.items():
        manifest_path = args.output_directory / scene_key / "manifest.json"
        if not manifest_path.exists():
            continue
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        atlas_scenes.append(
            {
                "scene": scene_key,
                "displayName": config["displayName"],
                "stageType": config["stageType"],
                "floorCount": manifest["floorCount"],
                "manifest": f"{scene_key}/manifest.json",
                "contactSheet": f"{scene_key}/{manifest['contactSheet']}",
            }
        )
    (args.output_directory / "atlas_manifest.json").write_text(
        json.dumps(
            {
                "schemaVersion": 1,
                "scenes": atlas_scenes,
            },
            ensure_ascii=False,
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )
    print(json.dumps(results, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
