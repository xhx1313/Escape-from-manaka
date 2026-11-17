from __future__ import annotations

import argparse
from collections import defaultdict
from pathlib import Path

import UnityPy


def path_id(reader) -> int:
    return int(getattr(getattr(reader, "object_reader", None), "path_id", 0))


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Print top-level scene transform hierarchy."
    )
    parser.add_argument("--game-data", type=Path, required=True)
    parser.add_argument("--level", required=True)
    parser.add_argument("--depth", type=int, default=4)
    args = parser.parse_args()

    environment = UnityPy.load(str(args.game_data))
    level_objects = [
        obj
        for obj in environment.objects
        if getattr(getattr(obj, "assets_file", None), "name", None)
        == args.level
    ]
    transforms: dict[int, dict] = {}
    children: dict[int, list[int]] = defaultdict(list)
    for reader in level_objects:
        if reader.type.name not in ("Transform", "RectTransform"):
            continue
        try:
            transform = reader.read()
            game_object = transform.m_GameObject.read()
            key = int(reader.path_id)
            father_id = int(transform.m_Father.m_PathID)
            transforms[key] = {
                "id": key,
                "name": str(game_object.m_Name),
                "father": father_id,
                "position": (
                    round(float(transform.m_LocalPosition.x), 3),
                    round(float(transform.m_LocalPosition.y), 3),
                    round(float(transform.m_LocalPosition.z), 3),
                ),
            }
            if father_id:
                children[father_id].append(key)
        except Exception:
            continue

    roots = [
        key
        for key, item in transforms.items()
        if not item["father"] or item["father"] not in transforms
    ]
    roots.sort(key=lambda key: transforms[key]["name"].lower())

    def print_tree(key: int, depth: int, prefix: str) -> None:
        item = transforms[key]
        descendants = len(children.get(key, []))
        print(
            f"{prefix}{item['name']} id={key} "
            f"local={item['position']} children={descendants}"
        )
        if depth >= args.depth:
            return
        for child_key in sorted(
            children.get(key, []),
            key=lambda child: transforms[child]["name"].lower(),
        ):
            print_tree(child_key, depth + 1, prefix + "  ")

    print(f"level={args.level} transforms={len(transforms)} roots={len(roots)}")
    for root in roots:
        print_tree(root, 0, "")


if __name__ == "__main__":
    main()
