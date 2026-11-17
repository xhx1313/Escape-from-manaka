from __future__ import annotations

import argparse
import json
import math
from pathlib import Path

import numpy as np
import UnityPy
from PIL import Image


DEFAULT_BOUNDS = {
    "minX": 6.19,
    "maxX": 155.544998,
    "minY": 49.001999,
    "maxY": 52.145,
    "minZ": 8.386999,
    "maxZ": 157.720001,
}
PARK_LEVEL = "level5"
SOURCE_SIZE = 2560
SURFACE_BELOW = 0.60
SURFACE_ABOVE = 0.80


def pointer_key(pointer) -> tuple[int, int]:
    return (id(pointer.assetsfile), int(pointer.m_PathID))


def read_pointed(pointer, cache: dict):
    if pointer is None or pointer.m_PathID == 0:
        return None
    key = pointer_key(pointer)
    if key not in cache:
        cache[key] = pointer.read()
    return cache[key]


def quaternion_matrix(rotation) -> np.ndarray:
    x = float(rotation.x)
    y = float(rotation.y)
    z = float(rotation.z)
    w = float(rotation.w)
    xx, yy, zz = x * x, y * y, z * z
    xy, xz, yz = x * y, x * z, y * z
    wx, wy, wz = w * x, w * y, w * z
    matrix = np.identity(4, dtype=np.float64)
    matrix[:3, :3] = np.array(
        [
            [1.0 - 2.0 * (yy + zz), 2.0 * (xy - wz), 2.0 * (xz + wy)],
            [2.0 * (xy + wz), 1.0 - 2.0 * (xx + zz), 2.0 * (yz - wx)],
            [2.0 * (xz - wy), 2.0 * (yz + wx), 1.0 - 2.0 * (xx + yy)],
        ],
        dtype=np.float64,
    )
    return matrix


def local_transform_matrix(transform) -> np.ndarray:
    position = transform.m_LocalPosition
    scale = transform.m_LocalScale
    matrix = quaternion_matrix(transform.m_LocalRotation)
    matrix[:3, 0] *= float(scale.x)
    matrix[:3, 1] *= float(scale.y)
    matrix[:3, 2] *= float(scale.z)
    matrix[:3, 3] = [
        float(position.x),
        float(position.y),
        float(position.z),
    ]
    return matrix


def world_transform_matrix(
    transform,
    cache: dict,
    visiting: set[int],
) -> np.ndarray:
    key = pointer_key_from_object(transform)
    if key in cache:
        return cache[key]
    if key in visiting:
        return np.identity(4, dtype=np.float64)
    visiting.add(key)
    local = local_transform_matrix(transform)
    parent_pointer = transform.m_Father
    if parent_pointer.m_PathID != 0:
        parent = parent_pointer.read()
        local = world_transform_matrix(parent, cache, visiting) @ local
    visiting.discard(key)
    cache[key] = local
    return local


def pointer_key_from_object(obj) -> tuple[int, int]:
    reader = getattr(obj, "object_reader", None)
    path_id = int(getattr(reader, "path_id", 0))
    return (id(obj.assets_file), path_id or id(obj))


def transform_points(
    positions: np.ndarray,
    matrix: np.ndarray,
) -> np.ndarray:
    if np.allclose(matrix, np.identity(4)):
        return positions
    homogeneous = np.ones((positions.shape[0], 4), dtype=np.float64)
    homogeneous[:, :3] = positions
    transformed = homogeneous @ matrix.T
    return transformed[:, :3]


def load_vertex_data(mesh, bundle_files):
    vertex_data = mesh.m_VertexData
    vertex_count = int(vertex_data.m_VertexCount)
    stream = mesh.m_StreamData
    if vertex_count <= 0:
        return None, None, None
    if stream is None or not stream.path:
        return None, None, None
    reader = bundle_files.get(stream.path)
    if reader is None:
        return None, None, None
    stream_size = int(stream.size)
    stream_offset = int(stream.offset)
    if stream_size <= 0 or stream_size % vertex_count:
        return None, None, None
    stride = stream_size // vertex_count
    raw = np.frombuffer(
        reader.bytes,
        dtype=np.uint8,
        count=stream_size,
        offset=stream_offset,
    ).reshape((vertex_count, stride))
    channels = vertex_data.m_Channels
    position_channel = channels[0]
    uv_channel = channels[4]
    if (
        position_channel.format != 0
        or position_channel.dimension < 3
        or position_channel.offset + 12 > stride
    ):
        return None, None, None
    positions = (
        raw[:, position_channel.offset : position_channel.offset + 12]
        .copy()
        .view(np.float32)
        .reshape((vertex_count, 3))
    )
    uvs = None
    uv_format_dtype = {
        0: np.float32,  # VertexAttributeFormat.Float32
        1: np.float16,  # VertexAttributeFormat.Float16
    }.get(int(uv_channel.format))
    if uv_format_dtype is not None and uv_channel.dimension >= 2:
        uv_item_size = np.dtype(uv_format_dtype).itemsize
        uv_bytes = uv_item_size * int(uv_channel.dimension)
        if uv_channel.offset + uv_bytes <= stride:
            decoded = (
                raw[
                    :,
                    uv_channel.offset : uv_channel.offset + uv_bytes,
                ]
                .copy()
                .view(uv_format_dtype)
                .reshape((vertex_count, int(uv_channel.dimension)))
            )
            uvs = decoded[:, :2].astype(np.float32)
    index_dtype = (
        np.uint16 if int(mesh.m_IndexFormat) == 0 else np.uint32
    )
    indices = np.frombuffer(
        bytes(mesh.m_IndexBuffer),
        dtype=index_dtype,
    )
    return positions, uvs, indices


def material_average(material) -> tuple[np.ndarray, np.ndarray | None, np.ndarray]:
    base = np.ones(4, dtype=np.float32)
    if material is None:
        return base[:3], None, np.ones(2, dtype=np.float32)
    properties = material.m_SavedProperties
    for name, color in properties.m_Colors:
        if name in ("_BaseColor", "_Color"):
            base = np.array(
                [color.r, color.g, color.b, color.a],
                dtype=np.float32,
            )
            break
    texture = None
    uv_scale = np.ones(2, dtype=np.float32)
    for name, texture_environment in properties.m_TexEnvs:
        if name not in ("_BaseMap", "_MainTex", "_Albedo", "_AlbedoMap"):
            continue
        pointer = texture_environment.m_Texture
        if pointer.m_PathID == 0:
            continue
        try:
            texture_object = pointer.read()
            texture = np.asarray(texture_object.image.convert("RGBA"))
            uv_scale = np.array(
                [
                    float(texture_environment.m_Scale.x),
                    float(texture_environment.m_Scale.y),
                ],
                dtype=np.float32,
            )
            break
        except Exception:
            continue
    return np.clip(base[:3], 0.0, 1.0), texture, uv_scale


def texture_colors(
    texture: np.ndarray | None,
    uv_scale: np.ndarray,
    uvs: np.ndarray,
    base: np.ndarray,
) -> np.ndarray:
    if uvs.size == 0:
        return np.empty((0, 3), dtype=np.float32)
    base = np.clip(np.asarray(base, dtype=np.float32), 0.0, 1.0)
    if texture is None:
        return np.broadcast_to(base, (uvs.shape[0], 3)).copy()

    width = texture.shape[1]
    height = texture.shape[0]
    u = np.mod(
        np.asarray(uvs[:, 0], dtype=np.float32) * float(uv_scale[0]),
        1.0,
    )
    v = np.mod(
        np.asarray(uvs[:, 1], dtype=np.float32) * float(uv_scale[1]),
        1.0,
    )
    source_x = u * float(width - 1)
    source_y = v * float(height - 1)
    x0 = np.floor(source_x).astype(np.int32)
    y0 = np.floor(source_y).astype(np.int32)
    x1 = np.minimum(x0 + 1, width - 1)
    y1 = np.minimum(y0 + 1, height - 1)
    tx = (source_x - x0)[:, None]
    ty = (source_y - y0)[:, None]
    top = (
        texture[y0, x0].astype(np.float32) * (1.0 - tx)
        + texture[y0, x1].astype(np.float32) * tx
    )
    bottom = (
        texture[y1, x0].astype(np.float32) * (1.0 - tx)
        + texture[y1, x1].astype(np.float32) * tx
    )
    sampled = top * (1.0 - ty) + bottom * ty
    opaque = sampled[:, 3] >= 24.0
    rgb = base[None, :] * sampled[:, :3] / 255.0
    return np.where(
        opaque[:, None],
        np.clip(rgb, 0.0, 1.0),
        base[None, :],
    ).astype(np.float32)


def texture_color(
    texture: np.ndarray | None,
    uv_scale: np.ndarray,
    uv: np.ndarray,
    base: np.ndarray,
) -> np.ndarray:
    return texture_colors(
        texture,
        uv_scale,
        np.asarray(uv, dtype=np.float32).reshape(1, 2),
        base,
    )[0]


def rasterize_triangle(
    triangle: np.ndarray,
    triangle_uv: np.ndarray,
    base_color: np.ndarray,
    texture: np.ndarray | None,
    uv_scale: np.ndarray,
    size: int,
    bounds: dict[str, float],
    walkable: np.ndarray,
    heights: np.ndarray,
    visible_heights: np.ndarray,
    output: np.ndarray,
    valid: np.ndarray,
) -> int:
    if (
        float(triangle[:, 0].max()) < bounds["minX"]
        or float(triangle[:, 0].min()) > bounds["maxX"]
        or float(triangle[:, 2].max()) < bounds["minZ"]
        or float(triangle[:, 2].min()) > bounds["maxZ"]
        or float(triangle[:, 1].max()) < bounds["minY"] - SURFACE_BELOW
        or float(triangle[:, 1].min()) > bounds["maxY"] + SURFACE_ABOVE
    ):
        return 0
    edge1 = triangle[1] - triangle[0]
    edge2 = triangle[2] - triangle[0]
    normal = np.cross(edge1, edge2)
    length = float(np.linalg.norm(normal))
    if length <= 1.0e-8 or abs(float(normal[1])) / length < 0.35:
        return 0
    source_x = (
        (triangle[:, 0] - bounds["minX"])
        / (bounds["maxX"] - bounds["minX"])
        * (size - 1)
    )
    source_y = (
        (bounds["maxZ"] - triangle[:, 2])
        / (bounds["maxZ"] - bounds["minZ"])
        * (size - 1)
    )
    x0 = max(0, int(math.floor(float(source_x.min()))))
    x1 = min(size - 1, int(math.ceil(float(source_x.max()))))
    y0 = max(0, int(math.floor(float(source_y.min()))))
    y1 = min(size - 1, int(math.ceil(float(source_y.max()))))
    if x1 < x0 or y1 < y0:
        return 0
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
    patch_walkable = walkable[y0 : y1 + 1, x0 : x1 + 1]
    patch_ground = heights[y0 : y1 + 1, x0 : x1 + 1]
    patch_heights = visible_heights[
        y0 : y1 + 1,
        x0 : x1 + 1,
    ]
    candidate = (
        inside
        & patch_walkable
        & (surface_y >= patch_ground - SURFACE_BELOW)
        & (surface_y <= patch_ground + SURFACE_ABOVE)
    )
    if not candidate.any():
        return 0
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
        triangle_uv[0][None, :] * weight_a[accepted, None]
        + triangle_uv[1][None, :] * weight_b[accepted, None]
        + triangle_uv[2][None, :] * weight_c[accepted, None]
    )
    colors = texture_colors(
        texture,
        uv_scale,
        accepted_uv,
        base_color,
    )
    output[absolute_y, absolute_x] = np.clip(
        colors * 255.0,
        0,
        255,
    ).astype(np.uint8)
    valid[absolute_y, absolute_x] = 1
    return count


def render_models(
    game_data: Path,
    walkable_path: Path,
    height_path: Path,
    output_directory: Path,
    size: int,
    bounds: dict[str, float],
) -> dict:
    environment = UnityPy.load(str(game_data))
    environment_objects = list(environment.objects)
    bundle = next(iter(environment.files.values()))
    bundle_files = bundle.files
    level_objects = [
        obj
        for obj in environment_objects
        if getattr(getattr(obj, "assets_file", None), "name", None)
        == PARK_LEVEL
    ]
    object_cache = {}
    transform_cache = {}
    mesh_cache = {}
    material_cache = {}

    walkable = np.fromfile(walkable_path, dtype=np.uint8).reshape(
        (SOURCE_SIZE, SOURCE_SIZE)
    ).astype(np.uint8)
    heights = np.fromfile(height_path, dtype=np.float32).reshape(
        (SOURCE_SIZE, SOURCE_SIZE)
    )
    if size != SOURCE_SIZE:
        walkable = np.asarray(
            Image.fromarray(walkable * 255, mode="L").resize(
                (size, size),
                Image.Resampling.NEAREST,
            ),
            dtype=np.uint8,
        ) > 0
        heights = np.asarray(
            Image.fromarray(heights, mode="F").resize(
                (size, size),
                Image.Resampling.NEAREST,
            ),
            dtype=np.float32,
        )
    else:
        walkable = walkable.astype(bool)
    output = np.zeros((size, size, 3), dtype=np.uint8)
    valid = np.zeros((size, size), dtype=np.uint8)
    visible_heights = np.full(
        (size, size),
        -1.0e20,
        dtype=np.float32,
    )

    renderers = [
        obj for obj in level_objects if obj.type.name == "MeshRenderer"
    ]
    rendered_renderers = 0
    rendered_submeshes = 0
    rendered_triangles = 0
    colored_pixels = 0
    for renderer_index, renderer_reader in enumerate(renderers, 1):
        try:
            renderer = renderer_reader.read()
            game_object = renderer.m_GameObject.read()
        except Exception:
            continue
        if (
            not bool(renderer.m_Enabled)
            or not bool(game_object.m_IsActive)
            or int(game_object.m_Layer) not in (0, 20)
            or game_object.m_Name.startswith("EFM_")
            or "Manustia" in game_object.m_Name
            or "NPC" in game_object.m_Name
        ):
            continue
        transform = None
        mesh_filter = None
        for component_pair in game_object.m_Component:
            try:
                component = component_pair.component.read()
            except Exception:
                continue
            name = type(component).__name__
            if name == "Transform":
                transform = component
            elif name == "MeshFilter":
                mesh_filter = component
        if transform is None or mesh_filter is None:
            continue
        mesh_pointer = mesh_filter.m_Mesh
        if mesh_pointer.m_PathID == 0:
            continue
        mesh_key = pointer_key(mesh_pointer)
        if mesh_key not in mesh_cache:
            try:
                mesh = mesh_pointer.read()
                mesh_cache[mesh_key] = (
                    mesh,
                    *load_vertex_data(mesh, bundle_files),
                )
            except Exception:
                mesh_cache[mesh_key] = (None, None, None, None)
        mesh, positions, uvs, indices = mesh_cache[mesh_key]
        if mesh is None or positions is None or indices is None:
            continue

        combined_mesh = mesh.m_Name.startswith("Combined Mesh (root: scene)")
        if combined_mesh:
            world_positions = positions.astype(np.float64, copy=False)
        else:
            matrix = world_transform_matrix(
                transform,
                transform_cache,
                set(),
            )
            world_positions = transform_points(positions, matrix)
        material_list = []
        for material_pointer in renderer.m_Materials:
            key = pointer_key(material_pointer)
            if key not in material_cache:
                try:
                    material_cache[key] = material_average(
                        material_pointer.read()
                    )
                except Exception:
                    material_cache[key] = (
                        np.ones(3, dtype=np.float32),
                        None,
                        np.ones(2, dtype=np.float32),
                    )
            material_list.append(material_cache[key])
        if not material_list:
            material_list = [
                (
                    np.ones(3, dtype=np.float32),
                    None,
                    np.ones(2, dtype=np.float32),
                )
            ]

        submeshes = mesh.m_SubMeshes
        if combined_mesh and renderer.m_StaticBatchInfo is not None:
            first = int(renderer.m_StaticBatchInfo.firstSubMesh)
            count = int(renderer.m_StaticBatchInfo.subMeshCount)
            subset = submeshes[first : first + count]
            subset_offset = first
        else:
            subset = submeshes
            subset_offset = 0
        if not subset:
            continue
        rendered_renderers += 1
        for local_index, submesh in enumerate(subset):
            submesh_index = subset_offset + local_index
            material = material_list[
                min(local_index, len(material_list) - 1)
            ]
            # A multi-material mesh maps its submesh index directly.
            if len(material_list) >= len(subset):
                material = material_list[local_index]
            dtype = np.uint16 if int(mesh.m_IndexFormat) == 0 else np.uint32
            start = int(submesh.firstByte) // np.dtype(dtype).itemsize
            triangle_indices = indices[
                start : start + int(submesh.indexCount)
            ]
            if triangle_indices.size < 3:
                continue
            rendered_submeshes += 1
            for offset in range(
                0,
                triangle_indices.size - 2,
                3,
            ):
                tri_indices = triangle_indices[offset : offset + 3].astype(
                    np.int64,
                    copy=False,
                )
                if tri_indices.max() >= world_positions.shape[0]:
                    continue
                triangle = world_positions[tri_indices]
                triangle_uv = (
                    uvs[tri_indices]
                    if uvs is not None
                    else np.zeros((3, 2), dtype=np.float32)
                )
                base_color, texture, uv_scale = material
                colored_pixels += rasterize_triangle(
                    triangle,
                    triangle_uv,
                    base_color,
                    texture,
                    uv_scale,
                    size,
                    bounds,
                    walkable,
                    heights,
                    visible_heights,
                    output,
                    valid,
                )
                rendered_triangles += 1
        if renderer_index % 250 == 0:
            print(
                f"renderers={renderer_index}/{len(renderers)} "
                f"submeshes={rendered_submeshes} "
                f"triangles={rendered_triangles} "
                f"pixels={colored_pixels}",
                flush=True,
            )

    output_directory.mkdir(parents=True, exist_ok=True)
    image_path = output_directory / "park_model_albedo_2560.png"
    Image.fromarray(output, mode="RGB").save(
        image_path,
        optimize=True,
    )
    preview_path = output_directory / "park_model_albedo_1024.png"
    Image.fromarray(output, mode="RGB").resize(
        (1024, 1024),
        Image.Resampling.LANCZOS,
    ).save(preview_path, optimize=True)
    valid_path = output_directory / "park_model_valid_2560.r8"
    valid.tofile(valid_path)
    metadata = {
        "type": "visual_mesh_albedo",
        "level": PARK_LEVEL,
        "renderers": rendered_renderers,
        "submeshes": rendered_submeshes,
        "triangles": rendered_triangles,
        "coloredPixels": colored_pixels,
        "modelCoverageRatio": float(
            (valid.astype(bool) & walkable).sum()
            / max(int(walkable.sum()), 1)
        ),
        "image": image_path.name,
        "preview": preview_path.name,
        "validMask": valid_path.name,
        **bounds,
        "imageNorthAxis": "+Z",
    }
    (output_directory / "park_model_albedo_2560.json").write_text(
        json.dumps(metadata, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    return metadata


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Rasterize the Park level meshes into an unlit albedo map."
    )
    parser.add_argument("--game-data", type=Path, required=True)
    parser.add_argument("--walkable", type=Path, required=True)
    parser.add_argument("--height", type=Path, required=True)
    parser.add_argument("--output-directory", type=Path, required=True)
    parser.add_argument("--size", type=int, default=2560)
    args = parser.parse_args()
    metadata = render_models(
        args.game_data,
        args.walkable,
        args.height,
        args.output_directory,
        args.size,
        DEFAULT_BOUNDS,
    )
    print(json.dumps(metadata, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
