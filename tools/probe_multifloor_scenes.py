from __future__ import annotations

import argparse
import json
from pathlib import Path

import numpy as np
import UnityPy

from rasterize_park_models import (
    load_vertex_data,
    pointer_key,
    transform_points,
    world_transform_matrix,
)


def probe_level(
    level_objects: list,
    bundle_files: dict,
    level_name: str,
) -> dict:
    transform_cache: dict = {}
    mesh_cache: dict = {}
    world_points: list[np.ndarray] = []
    horizontal_centres: list[np.ndarray] = []
    renderer_rows: list[list[object]] = []

    renderers = [
        obj for obj in level_objects if obj.type.name == "MeshRenderer"
    ]
    for renderer_reader in renderers:
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
            component_name = type(component).__name__
            if component_name == "Transform":
                transform = component
            elif component_name == "MeshFilter":
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
        mesh, positions, _uvs, indices = mesh_cache[mesh_key]
        if mesh is None or positions is None or indices is None or not indices.size:
            continue

        combined = mesh.m_Name.startswith("Combined Mesh (root: scene)")
        if combined:
            world = positions.astype(np.float64, copy=False)
        else:
            matrix = world_transform_matrix(
                transform,
                transform_cache,
                set(),
            )
            world = transform_points(positions, matrix)
        finite = np.isfinite(world).all(axis=1)
        world = world[finite]
        if not world.size:
            continue
        world_points.append(world)

        triangle_indices = indices[
            : indices.size - indices.size % 3
        ].reshape(-1, 3).astype(np.int64, copy=False)
        if triangle_indices.size:
            valid = triangle_indices.max(axis=1) < positions.shape[0]
            triangle_indices = triangle_indices[valid]
        if not triangle_indices.size:
            continue
        triangles = world[triangle_indices]
        normal = np.cross(
            triangles[:, 1] - triangles[:, 0],
            triangles[:, 2] - triangles[:, 0],
        )
        length = np.linalg.norm(normal, axis=1)
        upward = (
            (length > 1.0e-8)
            & (np.abs(normal[:, 1]) / np.maximum(length, 1.0e-8) > 0.35)
        )
        if upward.any():
            horizontal_centres.append(triangles[upward].mean(axis=1))
        renderer_rows.append(
            [
                str(game_object.m_Name),
                tuple(round(float(v), 3) for v in world.min(axis=0)),
                tuple(round(float(v), 3) for v in world.max(axis=0)),
            ]
        )

    all_points = np.concatenate(world_points, axis=0)
    centres = (
        np.concatenate(horizontal_centres, axis=0)
        if horizontal_centres
        else np.empty((0, 3), dtype=np.float64)
    )
    y_values = centres[:, 1]
    if y_values.size:
        floor_like = y_values[
            (y_values >= np.percentile(y_values, 0.5))
            & (y_values <= np.percentile(y_values, 99.5))
        ]
        histogram, edges = np.histogram(floor_like, bins=120)
        peaks = np.argsort(histogram)[-20:][::-1]
        peak_rows = [
            {
                "y": round(float((edges[i] + edges[i + 1]) * 0.5), 3),
                "count": int(histogram[i]),
            }
            for i in peaks
        ]
    else:
        peak_rows = []

    return {
        "level": level_name,
        "renderers": len(renderer_rows),
        "points": int(all_points.shape[0]),
        "horizontalTriangles": int(centres.shape[0]),
        "bounds": {
            "minX": float(all_points[:, 0].min()),
            "maxX": float(all_points[:, 0].max()),
            "minY": float(all_points[:, 1].min()),
            "maxY": float(all_points[:, 1].max()),
            "minZ": float(all_points[:, 2].min()),
            "maxZ": float(all_points[:, 2].max()),
        },
        "horizontalPercentiles": {
            str(q): round(float(np.percentile(y_values, q)), 3)
            for q in (0, 1, 5, 10, 25, 50, 75, 90, 95, 99, 100)
        }
        if y_values.size
        else {},
        "yPeaks": peak_rows,
        "largestRenderers": sorted(
            renderer_rows,
            key=lambda row: (
                (row[2][0] - row[1][0])
                * (row[2][2] - row[1][2])
            ),
            reverse=True,
        )[:30],
    }


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Probe scene bounds and floor-height histograms."
    )
    parser.add_argument("--game-data", type=Path, required=True)
    parser.add_argument("--levels", nargs="+", default=["level3", "level4", "level7"])
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()

    environment = UnityPy.load(str(args.game_data))
    objects = list(environment.objects)
    bundle = next(iter(environment.files.values()))
    results = []
    for level_name in args.levels:
        level_objects = [
            obj
            for obj in objects
            if getattr(getattr(obj, "assets_file", None), "name", None)
            == level_name
        ]
        result = probe_level(level_objects, bundle.files, level_name)
        results.append(result)
        print(
            json.dumps(
                {
                    key: value
                    for key, value in result.items()
                    if key != "largestRenderers"
                },
                ensure_ascii=False,
            ),
            flush=True,
        )
    if args.output:
        args.output.write_text(
            json.dumps(results, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )


if __name__ == "__main__":
    main()
