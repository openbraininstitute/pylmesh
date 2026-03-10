#!/usr/bin/env python3

from __future__ import annotations

import argparse
from multiprocessing import cpu_count
from pathlib import Path

from joblib import Parallel, delayed
import pylmesh

# Optional tqdm
try:
    from tqdm import tqdm
except ImportError:
    def tqdm(iterable=None, **kwargs):
        return iterable


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Convert OBJ meshes to GLB in parallel."
    )
    parser.add_argument(
        "--input-path",
        type=Path,
        required=True,
        help="OBJ file or directory containing OBJ files.",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        required=True,
        help="Directory where GLB files will be written.",
    )
    parser.add_argument(
        "--workers",
        type=int,
        default=max(1, cpu_count() - 1),
        help="Number of worker processes.",
    )
    parser.add_argument(
        "--write-stats-md",
        action="store_true",
        help="Write a Markdown report comparing OBJ and GLB file sizes.",
    )
    return parser.parse_args()


def collect_obj_files(input_path: Path) -> list[Path]:
    if not input_path.exists():
        raise FileNotFoundError(f"Input path does not exist: {input_path}")

    if input_path.is_file():
        if input_path.suffix.lower() != ".obj":
            raise ValueError(f"Input file is not an OBJ file: {input_path}")
        return [input_path]

    return sorted(
        p for p in input_path.iterdir()
        if p.is_file() and p.suffix.lower() == ".obj"
    )


def format_bytes(num_bytes: int) -> str:
    units = ["B", "KB", "MB", "GB", "TB"]
    value = float(num_bytes)

    for unit in units:
        if value < 1024.0 or unit == units[-1]:
            if unit == "B":
                return f"{int(value)} {unit}"
            return f"{value:.1f} {unit}"
        value /= 1024.0

    return f"{num_bytes} B"


def compute_ratio(obj_size: int, glb_size: int) -> float:
    if glb_size == 0:
        return float("inf")
    return obj_size / glb_size


def compute_reduction(obj_size: int, glb_size: int) -> float:
    if obj_size == 0:
        return 0.0
    return 100.0 * (1.0 - (glb_size / obj_size))


def build_stats_markdown(successful_pairs: list[tuple[Path, Path]]) -> str:
    rows = []
    total_obj = 0
    total_glb = 0

    for obj_path, glb_path in successful_pairs:
        if not obj_path.exists() or not glb_path.exists():
            continue

        obj_size = obj_path.stat().st_size
        glb_size = glb_path.stat().st_size
        ratio = compute_ratio(obj_size, glb_size)
        reduction = compute_reduction(obj_size, glb_size)

        total_obj += obj_size
        total_glb += glb_size

        rows.append(
            {
                "name": obj_path.stem,
                "obj_size": obj_size,
                "glb_size": glb_size,
                "ratio": ratio,
                "reduction": reduction,
            }
        )

    rows.sort(key=lambda row: row["name"])

    overall_ratio = compute_ratio(total_obj, total_glb)
    overall_reduction = compute_reduction(total_obj, total_glb)

    lines = []
    lines.append("# Mesh Size Comparison (OBJ vs GLB)")
    lines.append("")
    lines.append("| Mesh ID | OBJ Size | GLB Size | Compression Ratio | Size Reduction |")
    lines.append("|---|---:|---:|---:|---:|")

    for row in rows:
        lines.append(
            f"| {row['name']} | "
            f"{format_bytes(row['obj_size'])} | "
            f"{format_bytes(row['glb_size'])} | "
            f"{row['ratio']:.1f}x | "
            f"{row['reduction']:.1f}% |"
        )

    lines.append("")
    lines.append("## Summary")
    lines.append("")
    lines.append("| Metric | Value |")
    lines.append("|---|---:|")
    lines.append(f"| Total OBJ Size | {format_bytes(total_obj)} |")
    lines.append(f"| Total GLB Size | {format_bytes(total_glb)} |")
    lines.append(f"| Overall Compression Ratio | {overall_ratio:.1f}x |")
    lines.append(f"| Overall Size Reduction | {overall_reduction:.1f}% |")
    lines.append(f"| Number of Matched Meshes | {len(rows)} |")

    return "\n".join(lines)


def write_stats_markdown(output_dir: Path, successful_pairs: list[tuple[Path, Path]]) -> Path | None:
    if not successful_pairs:
        return None

    markdown = build_stats_markdown(successful_pairs)
    output_path = output_dir / "mesh_size_comparison.md"
    output_path.write_text(markdown, encoding="utf-8")
    return output_path


def convert_obj_to_glb(obj_path: Path, output_dir: Path) -> tuple[Path, Path | None, str | None]:
    try:
        mesh = pylmesh.load_mesh(str(obj_path))
        glb_path = output_dir / f"{obj_path.stem}.glb"
        pylmesh.save_mesh(str(glb_path), mesh)
        return obj_path, glb_path, None
    except Exception as exc:
        return obj_path, None, str(exc)


def main() -> int:
    args = parse_args()

    try:
        obj_files = collect_obj_files(args.input_path)
    except Exception as exc:
        print(f"Error: {exc}")
        return 1

    if not obj_files:
        print(f"No OBJ files found in: {args.input_path}")
        return 1

    args.output_dir.mkdir(parents=True, exist_ok=True)

    print(f"Found {len(obj_files)} OBJ file(s).")
    print(f"Using {args.workers} worker(s).")
    print(f"Output directory: {args.output_dir}")

    results = Parallel(n_jobs=args.workers, backend="loky")(
        delayed(convert_obj_to_glb)(obj_path, args.output_dir)
        for obj_path in tqdm(obj_files, desc="Converting meshes")
    )

    success = 0
    failure = 0
    successful_pairs: list[tuple[Path, Path]] = []

    for obj_path, glb_path, error in results:
        if error is None and glb_path is not None:
            success += 1
            successful_pairs.append((obj_path, glb_path))
        else:
            failure += 1
            print(f"[FAIL] {obj_path}: {error}")

    print("\nDone.")
    print(f"Successful: {success}")
    print(f"Failed:     {failure}")

    if args.write_stats_md:
        stats_path = write_stats_markdown(args.output_dir, successful_pairs)
        if stats_path is not None:
            print(f"Stats Markdown written to: {stats_path}")
        else:
            print("Stats Markdown was not written because no successful OBJ/GLB pairs were found.")

    return 0 if failure == 0 else 2


if __name__ == "__main__":
    raise SystemExit(main())