#! /bin/env python

import argparse
import os
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from enum import Enum
from typing import NamedTuple


class DataType(Enum):
    INT = "int"
    LONG = "long"
    FLOAT = "float"
    DOUBLE = "double"


class Op(Enum):
    ADD = "+"
    MUL = "*"

    @property
    def slug(self) -> str:
        return "add" if self == Op.ADD else "mul"

    @property
    def identity(self) -> str:
        return "0" if self == Op.ADD else "1"


class BenchmarkConfig(NamedTuple):
    data_type: DataType
    op: Op | None = None


class TaskPayload(NamedTuple):
    core_id: int
    c_file: str
    config: BenchmarkConfig


class TaskResult(NamedTuple):
    config: BenchmarkConfig
    results: dict[str, float] | None


def run_config(task: TaskPayload) -> TaskResult:
    cfg = task.config

    op_slug = f"_{cfg.op.slug}" if cfg.op else ""
    bin_file = f"./test_runner_{cfg.data_type.value}{op_slug}"

    compile_cmd = ["gcc", "-Og", "-mavx2", f"-DDATA_T={cfg.data_type.value}"]

    if cfg.op:
        compile_cmd.extend([f"-DIDENT={cfg.op.identity}", f"-DOP={cfg.op.value}"])

    compile_cmd.extend([task.c_file, "-o", bin_file])

    if subprocess.run(compile_cmd, capture_output=True).returncode != 0:
        return TaskResult(cfg, None)

    run_res = subprocess.run(
        ["taskset", "-c", str(task.core_id), bin_file], capture_output=True, text=True
    )

    if os.path.exists(bin_file):
        os.remove(bin_file)

    if run_res.returncode != 0:
        return TaskResult(cfg, None)

    results: dict[str, float] = {}
    for line in run_res.stdout.splitlines():
        parts = [p.strip() for p in line.split("|")]
        if len(parts) == 3:
            try:
                results[parts[0]] = float(parts[2])
            except ValueError:
                continue

    return TaskResult(cfg, results)


def print_table(
    all_results: dict[BenchmarkConfig, dict[str, float]],
    method_names: list[str],
    with_ops: bool,
) -> None:
    width = 87 if with_ops else 55
    print("\n" + "=" * width)
    print(f"{'BENCHMARK RESULTS (CPE)':^{width}}")
    print("=" * width)

    if with_ops:
        print(f"| {'Method':<20}|{'Integer':^31}|{'Floating point':^31}|")
        print(f"| {'':<20}|{'int':^15}|{'long':^15}|{'float':^15}|{'double':^15}|")
        print(f"| {'':<20}" + "|   +   |   *   " * 4 + "|")
        print("+" + "-" * 21 + "+" + "-------+" * 8)
    else:
        print(f"| {'Method':<20}|{'int':^7}|{'long':^7}|{'float':^7}|{'double':^7}|")
        print("+" + "-" * 21 + "+" + "-------+" * 4)

    configs = (
        [BenchmarkConfig(dtype, op) for dtype in DataType for op in Op]
        if with_ops
        else [BenchmarkConfig(dtype, None) for dtype in DataType]
    )

    for m in method_names:
        row = f"| {m:<20}"
        for cfg in configs:
            val = all_results.get(cfg, {}).get(m)
            val_str = f"{val:.2f}" if val is not None else "-"
            row += f"|{val_str:^7}"
        print(row + "|")

    print("=" * width)


def main() -> None:
    parser = argparse.ArgumentParser(description="Run SIMD benchmarks concurrently.")
    parser.add_argument("c_file", type=str, help="Path to the C benchmark source file")
    parser.add_argument(
        "--with-ops",
        action="store_true",
        help="Include op variants in compilation and results",
    )
    args = parser.parse_args()

    if not os.path.isfile(args.c_file):
        print(f"Error: File '{args.c_file}' not found.", file=sys.stderr)
        sys.exit(1)

    configs = (
        [BenchmarkConfig(dtype, op) for dtype in DataType for op in Op]
        if args.with_ops
        else [BenchmarkConfig(dtype, None) for dtype in DataType]
    )

    all_results: dict[BenchmarkConfig, dict[str, float]] = {}
    method_names: list[str] = []

    tasks = [
        TaskPayload(core_id=i, c_file=args.c_file, config=cfg)
        for i, cfg in enumerate(configs)
    ]

    with ThreadPoolExecutor() as executor:
        for task_result in executor.map(run_config, tasks):
            cfg, res = task_result.config, task_result.results

            if res is None:
                op_str = f" ({cfg.op.value})" if cfg.op else ""
                print(
                    f"Task failed for {cfg.data_type.value}{op_str}",
                    file=sys.stderr,
                )
                sys.exit(1)

            all_results[cfg] = res

            for m in res:
                if m not in method_names:
                    method_names.append(m)

    print_table(all_results, method_names, args.with_ops)


if __name__ == "__main__":
    main()
