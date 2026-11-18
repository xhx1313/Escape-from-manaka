#!/usr/bin/env python3
"""Export, validate, and atomically apply batch equipment design data."""

from __future__ import annotations

import argparse
import copy
import csv
import json
import os
import random
import shutil
import sys
import tempfile
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path
from typing import Any, Iterable


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_CATALOG = ROOT / "assets" / "equipment" / "equipment_catalog.json"
DEFAULT_MAPPING = ROOT / "tools" / "equipment_design_series_map.json"

STAT_KEYS = (
    "perception",
    "conservatism",
    "normality",
    "strength",
    "speed",
    "climax_speed_base",
    "stamina_capacity_base",
    "stamina_recovery_base",
    "handcuff_proficiency_base",
    "reward_base",
)

# Every formal stat currently has a higher-is-better runtime direction.
BENEFIT_SIGNS = {key: 1 for key in STAT_KEYS}

DEPTH_BANDS = {
    0: {"active": (2, 4), "extreme": (4, 8), "peak": (1, 2)},
    1: {"active": (3, 5), "extreme": (6, 11), "peak": (2, 3)},
    2: {"active": (5, 7), "extreme": (9, 15), "peak": (3, 4)},
    3: {"active": (6, 8), "extreme": (12, 20), "peak": (4, 5)},
}

BALANCE_LOW_TARGET_KEYS = {
    "conservatism",
    "strength",
    "climax_speed_base",
}

WORKBOOK_BASE_FIELDS = (
    "id",
    "name",
    "series",
    "task_family",
    "series_depth",
    "theme",
    "categories",
    "role",
    "tier",
    "style_adjustment",
    "reference_price",
)
WORKBOOK_TRAILING_FIELDS = ("rationale",)
WORKBOOK_FIELDS = (
    WORKBOOK_BASE_FIELDS
    + STAT_KEYS
    + WORKBOOK_TRAILING_FIELDS
)


@dataclass(frozen=True)
class Issue:
    severity: str
    item_id: str
    message: str


@dataclass(frozen=True)
class CatalogContext:
    catalog: dict[str, Any]
    mapping: dict[str, Any]
    items: dict[str, dict[str, Any]]
    category_keys: tuple[str, ...]
    family_by_id: dict[str, dict[str, Any]]
    series_to_family: dict[str, str]
    default_series: tuple[str, ...]

    def family_for_series(self, series: str) -> str:
        return self.series_to_family.get(
            series,
            str(self.mapping["default_family"]),
        )

    def depth_for_series(self, series: str) -> int:
        family_id = self.family_for_series(series)
        return int(self.family_by_id[family_id]["depth"])

    def series_for_family(self, family_id: str) -> tuple[str, ...]:
        if family_id == self.mapping["default_family"]:
            return self.default_series
        return tuple(
            series
            for series in self.catalog["series_order"]
            if self.family_for_series(series) == family_id
        )


def read_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except FileNotFoundError as exc:
        raise ValueError(f"文件不存在：{path}") from exc
    except json.JSONDecodeError as exc:
        raise ValueError(f"JSON 格式错误：{path}: {exc}") from exc
    if not isinstance(value, dict):
        raise ValueError(f"JSON 根节点必须是对象：{path}")
    return value


def atomic_write_json(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    payload = json.dumps(value, ensure_ascii=False, indent=2) + "\n"
    descriptor, temp_name = tempfile.mkstemp(
        prefix=path.name + ".",
        suffix=".tmp",
        dir=path.parent,
    )
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8", newline="\n") as handle:
            handle.write(payload)
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temp_name, path)
    except Exception:
        try:
            os.unlink(temp_name)
        except FileNotFoundError:
            pass
        raise


def load_context(catalog_path: Path, mapping_path: Path) -> tuple[CatalogContext, list[Issue]]:
    catalog = read_json(catalog_path)
    mapping = read_json(mapping_path)
    issues: list[Issue] = []

    if catalog.get("schema_version") != 5:
        issues.append(Issue("error", "<catalog>", "schema_version 必须为 5"))
    if tuple(catalog.get("stat_order", ())) != STAT_KEYS:
        issues.append(Issue("error", "<catalog>", "stat_order 与运行时契约不一致"))
    if tuple(catalog.get("stats", {}).keys()) != STAT_KEYS:
        issues.append(Issue("error", "<catalog>", "stats 定义缺失或顺序不一致"))

    category_keys: list[str] = []
    for category in catalog.get("categories", []):
        if not isinstance(category, dict) or "key" not in category:
            issues.append(Issue("error", "<catalog>", "categories 含非法条目"))
            continue
        category_keys.append(str(category["key"]))

    raw_items = catalog.get("items")
    if not isinstance(raw_items, dict):
        raise ValueError("equipment_catalog.json 的 items 必须是对象")
    items = {
        str(item_id): item
        for item_id, item in raw_items.items()
        if isinstance(item, dict)
    }
    if len(items) != len(raw_items):
        issues.append(Issue("error", "<catalog>", "items 含非对象条目"))

    if mapping.get("schema_version") != 1:
        issues.append(Issue("error", "<mapping>", "映射 schema_version 必须为 1"))
    default_family = str(mapping.get("default_family", ""))
    family_by_id = mapping.get("task_families")
    raw_series_map = mapping.get("equipment_series")
    if not isinstance(family_by_id, dict):
        raise ValueError("映射文件缺少 task_families 对象")
    if not isinstance(raw_series_map, dict):
        raise ValueError("映射文件缺少 equipment_series 对象")
    if default_family not in family_by_id:
        issues.append(Issue("error", "<mapping>", "default_family 未定义"))

    known_tags = {"eye_mask", "handcuff", "vibrator", "dildo"}
    for family_id, family in family_by_id.items():
        if not isinstance(family, dict):
            issues.append(Issue("error", "<mapping>", f"{family_id} 不是对象"))
            continue
        label = family.get("label")
        tags = family.get("required_tool_tags")
        depth = family.get("depth")
        if not isinstance(label, str) or not label.strip():
            issues.append(Issue("error", "<mapping>", f"{family_id} 缺少 label"))
        if not isinstance(tags, list) or any(
            not isinstance(tag, str) for tag in tags
        ):
            issues.append(
                Issue("error", "<mapping>", f"{family_id} required_tool_tags 非字符串数组")
            )
            tags = []
        unknown_tags = sorted(set(tags) - known_tags)
        if unknown_tags:
            issues.append(
                Issue(
                    "error",
                    "<mapping>",
                    f"{family_id} 含未知任务标签：{unknown_tags}",
                )
            )
        if isinstance(depth, bool) or not isinstance(depth, int) or depth not in DEPTH_BANDS:
            issues.append(Issue("error", "<mapping>", f"{family_id} depth 非法"))
        elif depth != len(tags):
            issues.append(
                Issue(
                    "error",
                    "<mapping>",
                    f"{family_id} depth={depth} 与任务标签数 {len(tags)} 不一致",
                )
            )

    catalog_series = tuple(str(series) for series in catalog.get("series_order", []))
    series_set = set(catalog_series)
    series_to_family: dict[str, str] = {}
    for series, family_id in raw_series_map.items():
        series_name = str(series)
        family_name = str(family_id)
        if series_name not in series_set:
            issues.append(
                Issue(
                    "error",
                    "<mapping>",
                    f"映射包含目录中不存在的装备系列：{series_name}",
                )
            )
        if family_name not in family_by_id:
            issues.append(
                Issue(
                    "error",
                    "<mapping>",
                    f"{series_name} 绑定了未知任务系列：{family_name}",
                )
            )
        if family_name == default_family:
            issues.append(
                Issue(
                    "warning",
                    "<mapping>",
                    f"{series_name} 无需显式绑定默认系列",
                )
            )
        series_to_family[series_name] = family_name

    default_series = tuple(
        series for series in catalog_series if series not in series_to_family
    )
    for family_id in family_by_id:
        if family_id == default_family:
            continue
        if not any(value == family_id for value in series_to_family.values()):
            issues.append(
                Issue(
                    "error",
                    "<mapping>",
                    f"任务系列 {family_id} 没有任何装备奖励系列",
                )
            )

    context = CatalogContext(
        catalog=catalog,
        mapping=mapping,
        items=items,
        category_keys=tuple(category_keys),
        family_by_id={
            str(key): value
            for key, value in family_by_id.items()
            if isinstance(value, dict)
        },
        series_to_family=series_to_family,
        default_series=default_series,
    )
    return context, issues


def stat_metrics(stats: dict[str, Any]) -> dict[str, int]:
    values = [int(stats[key]) for key in STAT_KEYS]
    return {
        "net": sum(values),
        "extreme": sum(abs(value) for value in values),
        "active": sum(value != 0 for value in values),
        "peak": max((abs(value) for value in values), default=0),
    }


def suggested_price(metrics: dict[str, int], depth: int, style_adjustment: int) -> int:
    raw = (
        1000
        + metrics["extreme"] * 180
        + metrics["active"] * 100
        + metrics["peak"] * 150
        + depth * 100
        + style_adjustment
    )
    rounded = ((raw + 50) // 100) * 100
    return max(1000, min(8000, rounded))


def validate_item(
    context: CatalogContext,
    item_id: str,
    item: dict[str, Any],
    *,
    style_adjustments: dict[str, int] | None = None,
) -> list[Issue]:
    issues: list[Issue] = []
    expected_id = item.get("id")
    if expected_id != item_id:
        issues.append(Issue("error", item_id, f"id 字段为 {expected_id!r}，与键不一致"))

    name = item.get("name")
    description = item.get("description")
    if not isinstance(name, str) or not name.strip():
        issues.append(Issue("error", item_id, "name 为空"))
    if not isinstance(description, str) or not description.strip():
        issues.append(Issue("error", item_id, "description 为空"))

    series = item.get("series")
    if series not in context.catalog.get("series_order", []):
        issues.append(Issue("error", item_id, f"未知装备系列：{series!r}"))
        depth = 0
    else:
        depth = context.depth_for_series(str(series))

    theme = item.get("theme")
    if theme not in context.catalog.get("themes", []):
        issues.append(Issue("error", item_id, f"未知主题：{theme!r}"))

    categories = item.get("categories")
    if not isinstance(categories, list) or not categories:
        issues.append(Issue("error", item_id, "categories 必须是非空数组"))
        categories = []
    unknown_categories = sorted(set(categories) - set(context.category_keys))
    if unknown_categories:
        issues.append(
            Issue("error", item_id, f"含未知类别：{unknown_categories}")
        )

    stats = item.get("stats")
    if not isinstance(stats, dict):
        issues.append(Issue("error", item_id, "stats 必须是对象"))
        return issues
    if tuple(stats.keys()) != STAT_KEYS:
        issues.append(
            Issue(
                "error",
                item_id,
                "stats 键或顺序与正式属性顺序不一致",
            )
        )
        return issues

    values: list[int] = []
    for key in STAT_KEYS:
        value = stats[key]
        if isinstance(value, bool) or not isinstance(value, int):
            issues.append(Issue("error", item_id, f"{key} 必须是整数"))
            continue
        if value < -5 or value > 5:
            issues.append(Issue("error", item_id, f"{key}={value} 超出 [-5, 5]"))
        values.append(value)
    if len(values) != len(STAT_KEYS):
        return issues

    if "prop" in categories:
        nonzero = {
            key: stats[key]
            for key in STAT_KEYS
            if stats[key] != 0
        }
        if nonzero:
            issues.append(
                Issue("error", item_id, f"道具类属性必须全零，当前为 {nonzero}")
            )

    metrics = stat_metrics(stats)
    if not 0 <= metrics["net"] <= 2:
        issues.append(
            Issue(
                "error",
                item_id,
                f"netBase={metrics['net']} 超出 [0, 2]",
            )
        )

    if metrics["active"] > 0 and "prop" not in categories:
        band = DEPTH_BANDS[depth]
        for metric_name, (minimum, maximum) in band.items():
            value = metrics[metric_name]
            if value < minimum or value > maximum:
                issues.append(
                    Issue(
                        "warning",
                        item_id,
                        f"depth={depth} 的 {metric_name}={value} "
                        f"不在目标区间 [{minimum}, {maximum}]",
                    )
                )
        if depth == 0 and metrics["peak"] >= 2 and not any(
            value < 0 for value in values
        ):
            issues.append(
                Issue(
                    "warning",
                    item_id,
                    "默认系列峰值达到 2 时至少应有一项负面代价",
                )
            )
        if depth > 0 and not any(value < 0 for value in values):
            issues.append(
                Issue(
                    "warning",
                    item_id,
                    "任务系列装备至少应有一项明确负面代价",
                )
            )

    price = item.get("reference_price")
    if isinstance(price, bool) or not isinstance(price, int):
        issues.append(Issue("error", item_id, "reference_price 必须是整数"))
    else:
        if price < 0:
            issues.append(Issue("error", item_id, "reference_price 不能为负数"))
        elif price % 100 != 0:
            issues.append(
                Issue("error", item_id, f"reference_price={price} 不是 100 的整数倍")
            )
        if (
            price > 0
            and metrics["active"] > 0
            and style_adjustments is not None
            and item_id in style_adjustments
        ):
            adjustment = style_adjustments[item_id]
            expected = suggested_price(metrics, depth, adjustment)
            if price != expected:
                issues.append(
                    Issue(
                        "warning",
                        item_id,
                        f"参考价 {price} 与公式建议 {expected} 不同（允许风格调整）",
                    )
                )

    return issues


def validate_catalog(
    context: CatalogContext,
    *,
    style_adjustments: dict[str, int] | None = None,
) -> list[Issue]:
    issues: list[Issue] = []
    zero_stat_items: list[str] = []
    zero_price_items: list[str] = []

    for item_id, item in sorted(context.items.items()):
        item_issues = validate_item(
            context,
            item_id,
            item,
            style_adjustments=style_adjustments,
        )
        issues.extend(item_issues)
        stats = item.get("stats")
        if (
            isinstance(stats, dict)
            and tuple(stats.keys()) == STAT_KEYS
            and all(stats[key] == 0 for key in STAT_KEYS)
            and "prop" not in item.get("categories", [])
        ):
            zero_stat_items.append(item_id)
        if item.get("reference_price") == 0:
            zero_price_items.append(item_id)

    if zero_stat_items:
        issues.append(
            Issue(
                "warning",
                "<catalog>",
                f"仍有 {len(zero_stat_items)} 件可带属性的物品十项属性全为 0",
            )
        )
    if zero_price_items:
        issues.append(
            Issue(
                "warning",
                "<catalog>",
                f"仍有 {len(zero_price_items)} 件物品参考价为 0",
            )
        )

    active_items = {
        item_id: item
        for item_id, item in context.items.items()
        if "prop" not in item.get("categories", [])
        and isinstance(item.get("stats"), dict)
        and tuple(item["stats"].keys()) == STAT_KEYS
        and all(
            isinstance(item["stats"][key], int)
            and not isinstance(item["stats"][key], bool)
            for key in STAT_KEYS
        )
        and any(item["stats"][key] != 0 for key in STAT_KEYS)
    }
    item_ids = sorted(active_items)
    for left_index, left_id in enumerate(item_ids):
        left = active_items[left_id]
        left_categories = set(left.get("categories", []))
        left_advantages = [
            int(left["stats"][key]) * BENEFIT_SIGNS[key]
            for key in STAT_KEYS
        ]
        for right_id in item_ids[left_index + 1 :]:
            right = active_items[right_id]
            shared = sorted(left_categories & set(right.get("categories", [])))
            if not shared:
                continue
            right_advantages = [
                int(right["stats"][key]) * BENEFIT_SIGNS[key]
                for key in STAT_KEYS
            ]
            if all(
                right_value >= left_value
                for left_value, right_value in zip(
                    left_advantages,
                    right_advantages,
                )
            ) and any(
                right_value > left_value
                for left_value, right_value in zip(
                    left_advantages,
                    right_advantages,
                )
            ):
                issues.append(
                    Issue(
                        "error",
                        right_id,
                        f"在类别 {shared} 中全面占优 {left_id}",
                    )
                )
            elif all(
                left_value >= right_value
                for left_value, right_value in zip(
                    left_advantages,
                    right_advantages,
                )
            ) and any(
                left_value > right_value
                for left_value, right_value in zip(
                    left_advantages,
                    right_advantages,
                )
            ):
                issues.append(
                    Issue(
                        "error",
                        left_id,
                        f"在类别 {shared} 中全面占优 {right_id}",
                    )
                )

    for left_index, left_id in enumerate(item_ids):
        left = active_items[left_id]
        left_vector = tuple(int(left["stats"][key]) for key in STAT_KEYS)
        left_categories = set(left.get("categories", []))
        for right_id in item_ids[left_index + 1 :]:
            right = active_items[right_id]
            right_vector = tuple(int(right["stats"][key]) for key in STAT_KEYS)
            distance = sum(
                abs(left_value - right_value)
                for left_value, right_value in zip(left_vector, right_vector)
            )
            shared_categories = sorted(
                left_categories & set(right.get("categories", []))
            )
            if distance == 0:
                issues.append(
                    Issue(
                        "error",
                        "<catalog>",
                        f"{left_id} 与 {right_id} 使用完全相同的属性向量 "
                        f"{left_vector}",
                    )
                )
            elif shared_categories and distance <= 2:
                issues.append(
                    Issue(
                        "warning",
                        "<catalog>",
                        f"{left_id} 与 {right_id} 在类别 {shared_categories} "
                        f"中数值过于相似（L1={distance}）",
                    )
                )

    return issues


def export_workbook(context: CatalogContext, output_path: Path) -> None:
    rows: list[dict[str, Any]] = []
    for item_id, item in sorted(
        context.items.items(),
        key=lambda pair: (
            context.catalog["series_order"].index(pair[1]["series"]),
            str(pair[1].get("name", "")),
            pair[0],
        ),
    ):
        series = str(item["series"])
        family_id = context.family_for_series(series)
        stats = item["stats"]
        row: dict[str, Any] = {
            "id": item_id,
            "name": item.get("name", ""),
            "series": series,
            "task_family": family_id,
            "series_depth": context.depth_for_series(series),
            "theme": item.get("theme", ""),
            "categories": "|".join(item.get("categories", [])),
            "role": "",
            "tier": "",
            "style_adjustment": 0,
            "reference_price": item.get("reference_price", 0),
            "rationale": "",
        }
        for key in STAT_KEYS:
            row[key] = stats.get(key, 0)
        rows.append(row)

    output_path.parent.mkdir(parents=True, exist_ok=True)
    descriptor, temp_name = tempfile.mkstemp(
        prefix=output_path.name + ".",
        suffix=".tmp",
        dir=output_path.parent,
    )
    try:
        with os.fdopen(
            descriptor,
            "w",
            encoding="utf-8-sig",
            newline="",
        ) as handle:
            writer = csv.DictWriter(handle, fieldnames=WORKBOOK_FIELDS)
            writer.writeheader()
            writer.writerows(rows)
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temp_name, output_path)
    except Exception:
        try:
            os.unlink(temp_name)
        except FileNotFoundError:
            pass
        raise


def parse_int(
    row: dict[str, str],
    field: str,
    item_id: str,
    issues: list[Issue],
) -> int:
    raw = (row.get(field) or "").strip()
    try:
        value = int(raw)
    except ValueError:
        issues.append(Issue("error", item_id, f"{field} 必须是整数，当前为 {raw!r}"))
        return 0
    return value


def workbook_overlay(
    context: CatalogContext,
    workbook_path: Path,
) -> tuple[dict[str, dict[str, Any]], dict[str, int], list[Issue]]:
    issues: list[Issue] = []
    overlays: dict[str, dict[str, Any]] = {}
    style_adjustments: dict[str, int] = {}
    try:
        with workbook_path.open("r", encoding="utf-8-sig", newline="") as handle:
            reader = csv.DictReader(handle)
            if tuple(reader.fieldnames or ()) != WORKBOOK_FIELDS:
                raise ValueError(
                    "CSV 表头不符合设计工作簿格式；请先运行 export，"
                    "不要调整列顺序或删除列"
                )
            rows = list(reader)
    except FileNotFoundError as exc:
        raise ValueError(f"工作簿不存在：{workbook_path}") from exc

    seen_ids: set[str] = set()
    for line_number, row in enumerate(rows, start=2):
        item_id = (row.get("id") or "").strip()
        if not item_id:
            issues.append(Issue("error", f"<line {line_number}>", "id 为空"))
            continue
        if item_id in seen_ids:
            issues.append(Issue("error", item_id, "工作簿中 id 重复"))
            continue
        seen_ids.add(item_id)
        if item_id not in context.items:
            issues.append(Issue("error", item_id, "目录中不存在该 id"))
            continue

        original = context.items[item_id]
        expected_series = str(original["series"])
        expected_family = context.family_for_series(expected_series)
        expected_depth = context.depth_for_series(expected_series)
        if (row.get("series") or "").strip() != expected_series:
            issues.append(
                Issue(
                    "error",
                    item_id,
                    f"series 是只读字段，必须为 {expected_series!r}",
                )
            )
        if (row.get("task_family") or "").strip() != expected_family:
            issues.append(
                Issue(
                    "error",
                    item_id,
                    f"task_family 是只读字段，必须为 {expected_family!r}",
                )
            )
        depth = parse_int(row, "series_depth", item_id, issues)
        if depth != expected_depth:
            issues.append(
                Issue(
                    "error",
                    item_id,
                    f"series_depth 是只读字段，必须为 {expected_depth}",
                )
            )

        style_adjustment = parse_int(row, "style_adjustment", item_id, issues)
        if style_adjustment < -200 or style_adjustment > 300:
            issues.append(
                Issue(
                    "error",
                    item_id,
                    f"style_adjustment={style_adjustment} 超出 [-200, 300]",
                )
            )
        style_adjustments[item_id] = style_adjustment

        overlay = copy.deepcopy(original)
        theme = (row.get("theme") or "").strip()
        if theme:
            overlay["theme"] = theme
        overlay["reference_price"] = parse_int(
            row,
            "reference_price",
            item_id,
            issues,
        )
        stats: dict[str, int] = {}
        for key in STAT_KEYS:
            stats[key] = parse_int(row, key, item_id, issues)
        overlay["stats"] = stats
        overlays[item_id] = overlay

    return overlays, style_adjustments, issues


def merged_catalog(
    context: CatalogContext,
    overlays: dict[str, dict[str, Any]],
) -> dict[str, Any]:
    merged = copy.deepcopy(context.catalog)
    for item_id, overlay in overlays.items():
        merged["items"][item_id] = copy.deepcopy(overlay)
    return merged


def context_with_catalog(
    context: CatalogContext,
    catalog: dict[str, Any],
) -> CatalogContext:
    items = {
        str(item_id): item
        for item_id, item in catalog["items"].items()
        if isinstance(item, dict)
    }
    return CatalogContext(
        catalog=catalog,
        mapping=context.mapping,
        items=items,
        category_keys=context.category_keys,
        family_by_id=context.family_by_id,
        series_to_family=context.series_to_family,
        default_series=context.default_series,
    )


def stat_totals(items: dict[str, dict[str, Any]]) -> dict[str, int]:
    return {
        key: sum(int(item["stats"][key]) for item in items.values())
        for key in STAT_KEYS
    }


def balanced_target_totals(
    current_totals: dict[str, int],
) -> dict[str, int]:
    total = sum(current_totals.values())
    lower, higher = divmod(total, len(STAT_KEYS))
    high_count = len(STAT_KEYS) - len(BALANCE_LOW_TARGET_KEYS)
    if higher != high_count:
        raise ValueError(
            "当前全局总属性不符合平衡目标："
            f"总和 {total}，十项应为 {high_count} 项 {lower + 1} "
            f"和 {len(BALANCE_LOW_TARGET_KEYS)} 项 {lower}"
        )
    return {
        key: (
            lower
            if key in BALANCE_LOW_TARGET_KEYS
            else lower + 1
        )
        for key in STAT_KEYS
    }


def _local_balance_issues(
    context: CatalogContext,
    item_id: str,
    stats: dict[str, int],
) -> list[Issue]:
    item = dict(context.items[item_id])
    item["stats"] = stats
    return [
        issue
        for issue in validate_item(context, item_id, item)
        if issue.severity in {"error", "warning"}
    ]


def _global_pair_issues(
    items: dict[str, dict[str, Any]],
    item_id: str,
    stats: dict[str, int],
) -> list[str]:
    issues: list[str] = []
    changed_categories = set(items[item_id].get("categories", []))
    changed_vector = tuple(stats[key] for key in STAT_KEYS)
    changed_advantages = tuple(
        stats[key] * BENEFIT_SIGNS[key] for key in STAT_KEYS
    )
    for other_id, other in items.items():
        if other_id == item_id or "prop" in other.get("categories", []):
            continue
        other_stats = other.get("stats")
        if not isinstance(other_stats, dict):
            continue
        if any(
            isinstance(other_stats.get(key), bool)
            or not isinstance(other_stats.get(key), int)
            for key in STAT_KEYS
        ):
            continue
        if not any(other_stats[key] != 0 for key in STAT_KEYS):
            continue

        other_vector = tuple(other_stats[key] for key in STAT_KEYS)
        distance = sum(
            abs(left - right)
            for left, right in zip(changed_vector, other_vector)
        )
        shared = sorted(
            changed_categories & set(other.get("categories", []))
        )
        if distance == 0:
            issues.append(f"与 {other_id} 属性向量完全相同")
            continue
        if shared and distance <= 2:
            issues.append(
                f"与 {other_id} 在类别 {shared} 中过于相似（L1={distance}）"
            )

        other_advantages = tuple(
            other_stats[key] * BENEFIT_SIGNS[key]
            for key in STAT_KEYS
        )
        if shared and all(
            right >= left
            for left, right in zip(changed_advantages, other_advantages)
        ) and any(
            right > left
            for left, right in zip(changed_advantages, other_advantages)
        ):
            issues.append(f"在类别 {shared} 中被 {other_id} 全面占优")
        if shared and all(
            left >= right
            for left, right in zip(changed_advantages, other_advantages)
        ) and any(
            left > right
            for left, right in zip(changed_advantages, other_advantages)
        ):
            issues.append(f"在类别 {shared} 中全面占优 {other_id}")
    return issues


def _balance_candidate_score(
    item: dict[str, Any],
    donor: str,
    receiver: str,
    touched: set[str],
    item_id: str,
) -> tuple[int, ...]:
    donor_value = int(item["stats"][donor])
    receiver_value = int(item["stats"][receiver])
    donor_abs = abs(donor_value)
    receiver_abs = abs(receiver_value)
    neutral_extreme = donor_value < 0 and receiver_value < 0
    exact_peak_swap = neutral_extreme and receiver_abs == donor_abs + 1
    avoids_peak = donor_abs + 1 <= max(
        abs(int(item["stats"][key]))
        for key in STAT_KEYS
        if key != receiver
    )
    return (
        item_id in touched,
        0 if exact_peak_swap else 1,
        0 if neutral_extreme else 1,
        0 if avoids_peak else 1,
        donor_abs + receiver_abs,
        donor,
        receiver,
        item_id,
    )


def plan_balanced_stat_totals(
    context: CatalogContext,
    *,
    seed_limit: int = 200,
) -> tuple[int, dict[str, dict[str, Any]], list[dict[str, Any]]]:
    current_totals = stat_totals(context.items)
    targets = balanced_target_totals(current_totals)
    if current_totals == targets:
        return 0, copy.deepcopy(context.items), []

    for seed in range(1, seed_limit + 1):
        rng = random.Random(seed)
        items = copy.deepcopy(context.items)
        donor_remaining = {
            key: current_totals[key] - targets[key]
            for key in STAT_KEYS
            if current_totals[key] > targets[key]
        }
        receiver_remaining = {
            key: targets[key] - current_totals[key]
            for key in STAT_KEYS
            if current_totals[key] < targets[key]
        }
        operations: list[dict[str, Any]] = []
        touched: set[str] = set()

        while donor_remaining or receiver_remaining:
            pairs = [
                (donor_need + receiver_need, donor, receiver)
                for donor, donor_need in donor_remaining.items()
                for receiver, receiver_need in receiver_remaining.items()
                if donor != receiver
            ]
            rng.shuffle(pairs)
            pairs.sort(reverse=True)

            selected: tuple[
                str,
                str,
                str,
                dict[str, int],
                dict[str, int],
            ] | None = None
            for _, donor, receiver in pairs:
                candidates: list[
                    tuple[
                        tuple[int, ...],
                        str,
                        dict[str, int],
                        dict[str, int],
                    ]
                ] = []
                for item_id in sorted(items):
                    item = items[item_id]
                    if "prop" in item.get("categories", []):
                        continue
                    if int(item["stats"][donor]) <= -5:
                        continue
                    if int(item["stats"][receiver]) >= 5:
                        continue
                    new_stats = dict(item["stats"])
                    new_stats[donor] = int(new_stats[donor]) - 1
                    new_stats[receiver] = int(new_stats[receiver]) + 1
                    candidates.append(
                        (
                            _balance_candidate_score(
                                item,
                                donor,
                                receiver,
                                touched,
                                item_id,
                            ),
                            item_id,
                            new_stats,
                            dict(item["stats"]),
                        )
                    )
                candidates.sort(key=lambda value: value[0])
                for _, item_id, new_stats, old_stats in candidates:
                    if _local_balance_issues(context, item_id, new_stats):
                        continue
                    if _global_pair_issues(items, item_id, new_stats):
                        continue
                    selected = (
                        item_id,
                        donor,
                        receiver,
                        new_stats,
                        old_stats,
                    )
                    break
                if selected is not None:
                    break

            if selected is None:
                break

            item_id, donor, receiver, new_stats, old_stats = selected
            items[item_id]["stats"] = new_stats
            touched.add(item_id)
            donor_remaining[donor] -= 1
            receiver_remaining[receiver] -= 1
            if donor_remaining[donor] == 0:
                del donor_remaining[donor]
            if receiver_remaining[receiver] == 0:
                del receiver_remaining[receiver]
            operations.append(
                {
                    "item_id": item_id,
                    "donor": donor,
                    "receiver": receiver,
                    "before": old_stats,
                    "after": new_stats,
                }
            )

        if donor_remaining or receiver_remaining:
            continue
        if stat_totals(items) != targets:
            continue
        balanced_context = context_with_catalog(
            context,
            merged_catalog(context, items),
        )
        issues = validate_catalog(balanced_context)
        if issues:
            continue
        return seed, items, operations

    raise ValueError("未找到满足全部装备设计约束的平衡方案")


def command_balance(args: argparse.Namespace) -> int:
    context, base_issues = load_context(args.catalog, args.mapping)
    base_errors = [issue for issue in base_issues if issue.severity == "error"]
    if base_errors:
        print_issues(base_errors)
        print("目录或映射存在硬错误，未进行平衡。")
        return 1

    seed, items, operations = plan_balanced_stat_totals(
        context,
        seed_limit=args.seed_limit,
    )
    before = stat_totals(context.items)
    after = stat_totals(items)
    touched = {str(operation["item_id"]) for operation in operations}
    labels = {
        key: str(context.catalog["stats"][key]["label"])
        for key in STAT_KEYS
    }
    print(
        f"平衡方案：seed={seed}，调整 {len(operations)} 个属性点，"
        f"涉及 {len(touched)} 件装备"
    )
    for key in STAT_KEYS:
        print(f"  {labels[key]:<8} {before[key]:+4d} -> {after[key]:+4d}")
    for operation in operations:
        item_id = str(operation["item_id"])
        donor = str(operation["donor"])
        receiver = str(operation["receiver"])
        print(
            f"  {item_id}: {labels[donor]} "
            f"{operation['before'][donor]:+d} -> "
            f"{operation['after'][donor]:+d}；{labels[receiver]} "
            f"{operation['before'][receiver]:+d} -> "
            f"{operation['after'][receiver]:+d}"
        )

    if not operations:
        print("十项总属性已经处于平衡目标。")
        return 0
    if not args.apply:
        print("dry-run：未写入 equipment_catalog.json；使用 --apply 执行写入。")
        return 0

    backup_dir = args.backup_dir
    if backup_dir is None:
        stamp = datetime.now().strftime("%Y%m%d-%H%M%S")
        backup_dir = ROOT / "backups" / f"equipment-balance-{stamp}"
    backup_dir.mkdir(parents=True, exist_ok=True)
    backup_path = backup_dir / "equipment_catalog.json"
    shutil.copy2(args.catalog, backup_path)
    balanced_catalog = merged_catalog(context, items)
    atomic_write_json(args.catalog, balanced_catalog)

    verify_context, verify_issues = load_and_validate(
        args.catalog,
        args.mapping,
    )
    if verify_issues:
        shutil.copy2(backup_path, args.catalog)
        print_issues(verify_issues)
        print("写入后校验失败，已从备份恢复目录。")
        return 1

    print(f"已写入平衡结果：{args.catalog}")
    print(f"原目录备份：{backup_path}")
    return 0


def print_issues(issues: Iterable[Issue], *, limit: int = 200) -> tuple[int, int]:
    issue_list = list(issues)
    errors = sum(issue.severity == "error" for issue in issue_list)
    warnings = sum(issue.severity == "warning" for issue in issue_list)
    for issue in issue_list[:limit]:
        print(f"[{issue.severity.upper():7}] {issue.item_id}: {issue.message}")
    remaining = len(issue_list) - limit
    if remaining > 0:
        print(f"... 其余 {remaining} 条问题未显示")
    print(f"结果：{errors} 个错误，{warnings} 个警告")
    return errors, warnings


def print_mapping(context: CatalogContext) -> None:
    labels = {
        family_id: str(family["label"])
        for family_id, family in context.family_by_id.items()
    }
    for series in context.catalog["series_order"]:
        family_id = context.family_for_series(series)
        print(
            f"{series:<12} -> {family_id:<26} "
            f"depth={context.depth_for_series(series)} "
            f"({labels[family_id]})"
        )
    print(f"默认装备系列数：{len(context.default_series)}")
    for series in context.default_series:
        print(f"  - {series}")


def load_and_validate(
    catalog_path: Path,
    mapping_path: Path,
) -> tuple[CatalogContext, list[Issue]]:
    context, mapping_issues = load_context(catalog_path, mapping_path)
    issues = list(mapping_issues)
    issues.extend(validate_catalog(context))
    return context, issues


def command_export(args: argparse.Namespace) -> int:
    context, issues = load_and_validate(args.catalog, args.mapping)
    hard_errors = [issue for issue in issues if issue.severity == "error"]
    if hard_errors:
        print_issues(hard_errors)
        print("目录存在硬错误，未导出工作簿。")
        return 1
    export_workbook(context, args.output)
    print(f"已导出 {len(context.items)} 件物品：{args.output}")
    return 0


def command_validate(args: argparse.Namespace) -> int:
    context, base_issues = load_context(args.catalog, args.mapping)
    style_adjustments: dict[str, int] = {}
    issues = list(base_issues)
    if args.input:
        overlays, style_adjustments, workbook_issues = workbook_overlay(
            context,
            args.input,
        )
        issues.extend(workbook_issues)
        validation_context = context_with_catalog(
            context,
            merged_catalog(context, overlays),
        )
    else:
        validation_context = context
    issues.extend(
        validate_catalog(
            validation_context,
            style_adjustments=style_adjustments,
        )
    )
    errors, warnings = print_issues(issues)
    if errors or (args.strict and warnings):
        return 1
    return 0


def command_apply(args: argparse.Namespace) -> int:
    context, base_issues = load_context(args.catalog, args.mapping)
    base_errors = [issue for issue in base_issues if issue.severity == "error"]
    if base_errors:
        print_issues(base_errors)
        print("目录或映射存在硬错误，未写入。")
        return 1

    overlays, style_adjustments, workbook_issues = workbook_overlay(
        context,
        args.input,
    )
    if workbook_issues:
        errors, _ = print_issues(workbook_issues)
        if errors:
            print("工作簿存在硬错误，未写入。")
            return 1
        if args.strict:
            print("严格模式下工作簿警告也会阻止写入。")
            return 1

    merged = merged_catalog(context, overlays)
    merged_context = context_with_catalog(context, merged)
    validation_issues = validate_catalog(
        merged_context,
        style_adjustments=style_adjustments,
    )
    errors, warnings = print_issues(validation_issues)
    if errors:
        print("合并后的目录存在硬错误，未写入。")
        return 1
    if args.strict and warnings:
        print("严格模式下警告会阻止写入。")
        return 1

    changed_ids: list[str] = []
    for item_id, overlay in overlays.items():
        original = context.items[item_id]
        if (
            original.get("theme") != overlay.get("theme")
            or original.get("reference_price") != overlay.get("reference_price")
            or original.get("stats") != overlay.get("stats")
        ):
            changed_ids.append(item_id)

    if not changed_ids:
        print("工作簿没有产生实际变更。")
        return 0
    print(f"待写入 {len(changed_ids)} 件物品：{', '.join(sorted(changed_ids))}")
    if args.dry_run:
        print("dry-run：未写入 equipment_catalog.json。")
        return 0

    backup_dir = args.backup_dir
    if backup_dir is None:
        stamp = datetime.now().strftime("%Y%m%d-%H%M%S")
        backup_dir = ROOT / "backups" / f"equipment-design-apply-{stamp}"
    backup_dir.mkdir(parents=True, exist_ok=True)
    backup_path = backup_dir / "equipment_catalog.json"
    shutil.copy2(args.catalog, backup_path)
    atomic_write_json(args.catalog, merged)

    verify_context, verify_issues = load_and_validate(
        args.catalog,
        args.mapping,
    )
    verify_errors = [
        issue for issue in verify_issues if issue.severity == "error"
    ]
    if verify_errors:
        shutil.copy2(backup_path, args.catalog)
        print_issues(verify_errors)
        print("写入后校验失败，已从备份恢复目录。")
        return 1

    print(f"已更新 {len(changed_ids)} 件物品。")
    print(f"原目录备份：{backup_path}")
    print(
        "后续构建会运行 tools/generate_equipment_data.py，"
        "自动生成原生装备数据。"
    )
    return 0


def command_map(args: argparse.Namespace) -> int:
    context, issues = load_context(args.catalog, args.mapping)
    print_mapping(context)
    if issues:
        print_issues(issues)
    return 1 if any(issue.severity == "error" for issue in issues) else 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Escape from Manaka 装备数值工作簿、校验和批量落盘工具",
    )
    parser.add_argument(
        "--catalog",
        type=Path,
        default=DEFAULT_CATALOG,
        help="装备目录 JSON",
    )
    parser.add_argument(
        "--mapping",
        type=Path,
        default=DEFAULT_MAPPING,
        help="任务系列与装备系列映射 JSON",
    )

    subparsers = parser.add_subparsers(dest="command", required=True)

    export_parser = subparsers.add_parser(
        "export",
        help="导出 CSV 设计工作簿",
    )
    export_parser.add_argument("-o", "--output", type=Path, required=True)
    export_parser.set_defaults(handler=command_export)

    validate_parser = subparsers.add_parser(
        "validate",
        help="校验当前目录或 CSV 工作簿",
    )
    validate_parser.add_argument("-i", "--input", type=Path)
    validate_parser.add_argument(
        "--strict",
        action="store_true",
        help="把警告也视为失败",
    )
    validate_parser.set_defaults(handler=command_validate)

    apply_parser = subparsers.add_parser(
        "apply",
        help="校验并事务写入工作簿",
    )
    apply_parser.add_argument("-i", "--input", type=Path, required=True)
    apply_parser.add_argument("--dry-run", action="store_true")
    apply_parser.add_argument("--strict", action="store_true")
    apply_parser.add_argument(
        "--backup-dir",
        type=Path,
        help="原目录备份目录；默认写入 backups/equipment-design-apply-*",
    )
    apply_parser.set_defaults(handler=command_apply)

    balance_parser = subparsers.add_parser(
        "balance",
        help="将十项全局总属性平衡到可达到的最近整数目标",
    )
    balance_parser.add_argument(
        "--apply",
        action="store_true",
        help="实际写入目录；默认只做 dry-run",
    )
    balance_parser.add_argument(
        "--seed-limit",
        type=int,
        default=200,
        help="确定性搜索的最大重试次数",
    )
    balance_parser.add_argument(
        "--backup-dir",
        type=Path,
        help="原目录备份目录；默认写入 backups/equipment-balance-*",
    )
    balance_parser.set_defaults(handler=command_balance)

    map_parser = subparsers.add_parser(
        "map",
        help="显示装备系列到任务系列的映射",
    )
    map_parser.set_defaults(handler=command_map)

    return parser


def main() -> int:
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    if hasattr(sys.stderr, "reconfigure"):
        sys.stderr.reconfigure(encoding="utf-8", errors="replace")
    parser = build_parser()
    args = parser.parse_args()
    try:
        return int(args.handler(args))
    except ValueError as exc:
        print(f"错误：{exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
