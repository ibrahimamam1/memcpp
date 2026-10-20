#!/usr/bin/env python3
"""
Benchmark Plotting Tool for memcpp

Plots the results of allocator benchmarks from CSV files:
1) Allocation & Deallocation latency vs request size curves:
   - Mean latency curves
   - Percentile curves: p50 (median), p95, and p99 (3-panel & individual plots)
   - Color-coded subjects, request size -1 labeled as 'Random'
2) Successive allocation latency vs number of allocations:
   - Time-series progression curves
   - Summary percentile comparison (p50, p95, p99) across subjects
3) External fragmentation plotted against request count.
"""

import os
import argparse
from pathlib import Path
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# Palette for consistent color coding across all experiments
SUBJECT_COLORS = {
    'malloc': '#1f77b4',     # Blue
    'first_fit': '#ff7f0e',   # Orange
    'best_fit': '#2ca02c',    # Green
    'next_fit': '#d62728',    # Red
}

SUBJECT_LABELS = {
    'malloc': 'malloc (glibc)',
    'first_fit': 'First Fit',
    'best_fit': 'Best Fit',
    'next_fit': 'Next Fit',
}

PERCENTILE_COLORS = {
    'p50': '#2ca02c',  # Green
    'p95': '#1f77b4',  # Blue
    'p99': '#d62728',  # Red
}

def resolve_paths(data_dir=None, output_dir=None):
    """Resolve data directory and output directory paths relative to repo root or bench."""
    base_dir = Path(__file__).resolve().parent
    repo_dir = base_dir.parent

    if data_dir is None:
        if (base_dir / "data").exists():
            data_path = base_dir / "data"
        elif (repo_dir / "bench" / "data").exists():
            data_path = repo_dir / "bench" / "data"
        else:
            data_path = base_dir / "data"
    else:
        data_path = Path(data_dir)

    if output_dir is None:
        output_path = base_dir / "plots"
    else:
        output_path = Path(output_dir)

    output_path.mkdir(parents=True, exist_ok=True)
    return data_path, output_path

def get_color(subject):
    return SUBJECT_COLORS.get(subject, None)

def get_label(subject):
    return SUBJECT_LABELS.get(subject, subject)

# =============================================================================
# 1) Allocation & Free Latency vs Request Size (Mean & Individual Percentiles)
# =============================================================================

def plot_latency_vs_request_size(csv_path, title, output_path, metric='mean', y_label=None, show=False):
    """
    Plots a latency vs request size curve for each subject.
    Request sizes: [16, 64, 256, 4096, -1], with -1 labeled as 'Random'.
    metric: 'mean', 'p50', 'p95', or 'p99'.
    """
    if not Path(csv_path).exists():
        print(f"Warning: {csv_path} not found. Skipping plot.")
        return None

    df = pd.read_csv(csv_path)

    size_order = [16, 64, 256, 4096, -1]
    size_labels = ['16 B', '64 B', '256 B', '4096 B', 'Random']
    x_indices = np.arange(len(size_order))
    size_to_x = {size: idx for idx, size in enumerate(size_order)}

    if metric == 'mean':
        agg = df.groupby(['subject', 'request_size'])['latency'].mean().reset_index()
        value_col = 'latency'
        default_ylabel = "Mean Latency (μs)"
    elif metric == 'p50':
        agg = df.groupby(['subject', 'request_size'])['latency'].quantile(0.50).reset_index()
        value_col = 'latency'
        default_ylabel = "Median Latency (p50, μs)"
    elif metric == 'p95':
        agg = df.groupby(['subject', 'request_size'])['latency'].quantile(0.95).reset_index()
        value_col = 'latency'
        default_ylabel = "95th Percentile Latency (p95, μs)"
    elif metric == 'p99':
        agg = df.groupby(['subject', 'request_size'])['latency'].quantile(0.99).reset_index()
        value_col = 'latency'
        default_ylabel = "99th Percentile Latency (p99, μs)"
    else:
        raise ValueError(f"Unknown metric: {metric}")

    fig, ax = plt.subplots(figsize=(8, 5), dpi=150)

    subjects = [s for s in ['malloc', 'first_fit', 'best_fit', 'next_fit'] if s in agg['subject'].unique()]
    for s in agg['subject'].unique():
        if s not in subjects:
            subjects.append(s)

    for subject in subjects:
        sub_df = agg[agg['subject'] == subject].copy()
        sub_df['sort_key'] = sub_df['request_size'].map(lambda x: size_to_x.get(x, 999))
        sub_df = sub_df.sort_values('sort_key')

        x_vals = [size_to_x[s] for s in sub_df['request_size'] if s in size_to_x]
        y_vals = sub_df[value_col].values

        color = get_color(subject)
        label = get_label(subject)

        ax.plot(x_vals, y_vals, marker='o', linewidth=2, markersize=7, 
                color=color, label=label)

    ax.set_xticks(x_indices)
    ax.set_xticklabels(size_labels, fontsize=11)
    ax.set_xlabel("Request Size", fontsize=12, fontweight='bold')
    ax.set_ylabel(y_label or default_ylabel, fontsize=12, fontweight='bold')
    ax.set_title(title, fontsize=14, fontweight='bold', pad=12)
    ax.grid(True, linestyle='--', alpha=0.6)
    ax.legend(frameon=True, fontsize=10, loc='best')

    plt.tight_layout()
    fig.savefig(output_path, dpi=300)
    print(f"Saved: {output_path}")
    if show:
        plt.show()
    plt.close(fig)
    return fig

# =============================================================================
# 2) 3-Panel Percentile Plots (p50, p95, p99 Side-by-Side)
# =============================================================================

def plot_latency_percentiles_panel(csv_path, title, output_path, show=False):
    """
    Plots a 3-panel figure showing p50, p95, and p99 curves side-by-side.
    """
    if not Path(csv_path).exists():
        print(f"Warning: {csv_path} not found. Skipping plot.")
        return None

    df = pd.read_csv(csv_path)

    size_order = [16, 64, 256, 4096, -1]
    size_labels = ['16 B', '64 B', '256 B', '4096 B', 'Random']
    size_to_x = {size: idx for idx, size in enumerate(size_order)}

    pcts = df.groupby(['subject', 'request_size'])['latency'].quantile([0.5, 0.95, 0.99]).unstack(level=-1).reset_index()
    pcts.columns = ['subject', 'request_size', 'p50', 'p95', 'p99']

    fig, axes = plt.subplots(1, 3, figsize=(16, 5), dpi=150, sharey=False)
    percentiles = [
        ('p50', 'p50 (Median) Latency'),
        ('p95', 'p95 (95th Percentile) Latency'),
        ('p99', 'p99 (99th Percentile) Latency'),
    ]

    subjects = [s for s in ['malloc', 'first_fit', 'best_fit', 'next_fit'] if s in pcts['subject'].unique()]
    for s in pcts['subject'].unique():
        if s not in subjects:
            subjects.append(s)

    for ax, (pct_col, pct_title) in zip(axes, percentiles):
        for subject in subjects:
            sub_df = pcts[pcts['subject'] == subject].copy()
            sub_df['sort_key'] = sub_df['request_size'].map(lambda x: size_to_x.get(x, 999))
            sub_df = sub_df.sort_values('sort_key')

            x_vals = [size_to_x[s] for s in sub_df['request_size'] if s in size_to_x]
            y_vals = sub_df[pct_col].values

            color = get_color(subject)
            label = get_label(subject)

            ax.plot(x_vals, y_vals, marker='o', linewidth=2, markersize=6, 
                    color=color, label=label)

        ax.set_xticks(range(len(size_order)))
        ax.set_xticklabels(size_labels, fontsize=10)
        ax.set_xlabel("Request Size", fontsize=11, fontweight='bold')
        ax.set_ylabel("Latency (μs)", fontsize=11, fontweight='bold')
        ax.set_title(pct_title, fontsize=12, fontweight='bold', pad=10)
        ax.grid(True, linestyle='--', alpha=0.6)
        ax.legend(frameon=True, fontsize=9, loc='best')

    fig.suptitle(title, fontsize=15, fontweight='bold', y=1.02)
    plt.tight_layout()
    fig.savefig(output_path, dpi=300, bbox_inches='tight')
    print(f"Saved: {output_path}")
    if show:
        plt.show()
    plt.close(fig)
    return fig

# =============================================================================
# High-Level Allocation & Deallocation Functions
# =============================================================================

def plot_alloc_latency(data_dir=None, output_dir=None, show=False):
    data_path, out_path = resolve_paths(data_dir, output_dir)
    csv_file = data_path / "alloc_latency.csv"
    return plot_latency_vs_request_size(
        csv_file,
        title="Allocation Latency vs Request Size (Mean)",
        output_path=out_path / "alloc_latency.png",
        metric='mean',
        show=show
    )

def plot_dealloc_latency(data_dir=None, output_dir=None, show=False):
    data_path, out_path = resolve_paths(data_dir, output_dir)
    csv_file = data_path / "dealloc_latency.csv"
    return plot_latency_vs_request_size(
        csv_file,
        title="Deallocation Latency vs Request Size (Mean)",
        output_path=out_path / "dealloc_latency.png",
        metric='mean',
        show=show
    )

def plot_alloc_latency_percentiles(data_dir=None, output_dir=None, show=False):
    data_path, out_path = resolve_paths(data_dir, output_dir)
    csv_file = data_path / "alloc_latency.csv"

    # 3-panel figure
    plot_latency_percentiles_panel(
        csv_file,
        title="Allocation Latency Percentiles (p50, p95, p99)",
        output_path=out_path / "alloc_latency_percentiles.png",
        show=show
    )

    # Individual percentile plots
    for pct in ['p50', 'p95', 'p99']:
        plot_latency_vs_request_size(
            csv_file,
            title=f"Allocation Latency vs Request Size ({pct.upper()})",
            output_path=out_path / f"alloc_latency_{pct}.png",
            metric=pct,
            show=show
        )

def plot_dealloc_latency_percentiles(data_dir=None, output_dir=None, show=False):
    data_path, out_path = resolve_paths(data_dir, output_dir)
    csv_file = data_path / "dealloc_latency.csv"

    # 3-panel figure
    plot_latency_percentiles_panel(
        csv_file,
        title="Deallocation Latency Percentiles (p50, p95, p99)",
        output_path=out_path / "dealloc_latency_percentiles.png",
        show=show
    )

    # Individual percentile plots
    for pct in ['p50', 'p95', 'p99']:
        plot_latency_vs_request_size(
            csv_file,
            title=f"Deallocation Latency vs Request Size ({pct.upper()})",
            output_path=out_path / f"dealloc_latency_{pct}.png",
            metric=pct,
            show=show
        )

# =============================================================================
# 3) Successive Allocation Latency vs Number of Allocations
# =============================================================================

def plot_latency_successive(data_dir=None, output_dir=None, rolling_window=25, show=False):
    """
    Plots successive allocation latency against the allocation number.
    """
    data_path, out_path = resolve_paths(data_dir, output_dir)
    csv_file = data_path / "alloc_latency_successive.csv"
    out_file = out_path / "alloc_latency_successive.png"

    if not csv_file.exists():
        print(f"Warning: {csv_file} not found. Skipping plot.")
        return None

    df = pd.read_csv(csv_file)

    fig, ax = plt.subplots(figsize=(10, 5.5), dpi=150)

    subjects = [s for s in ['malloc', 'first_fit', 'best_fit', 'next_fit'] if s in df['subject'].unique()]
    for s in df['subject'].unique():
        if s not in subjects:
            subjects.append(s)

    for subject in subjects:
        sub_df = df[df['subject'] == subject].copy().reset_index(drop=True)
        sub_df['alloc_number'] = sub_df.index + 1
        color = get_color(subject)
        label = get_label(subject)

        # Plot raw points lightly
        ax.plot(sub_df['alloc_number'], sub_df['latency'], 
                alpha=0.25, color=color, linewidth=0.8)

        # Plot smoothed moving average for clear trend visualization
        if len(sub_df) >= rolling_window:
            rolling_mean = sub_df['latency'].rolling(window=rolling_window, min_periods=1).mean()
            ax.plot(sub_df['alloc_number'], rolling_mean, 
                    linewidth=2.2, color=color, label=f"{label} (MA-{rolling_window})")
        else:
            ax.plot(sub_df['alloc_number'], sub_df['latency'], 
                    linewidth=1.8, color=color, label=label)

    ax.set_xlabel("Number of Allocations", fontsize=12, fontweight='bold')
    ax.set_ylabel("Latency (μs)", fontsize=12, fontweight='bold')
    ax.set_title("Successive Allocation Latency vs Number of Allocations", fontsize=14, fontweight='bold', pad=12)
    ax.grid(True, linestyle='--', alpha=0.6)
    ax.legend(frameon=True, fontsize=10, loc='best')

    plt.tight_layout()
    fig.savefig(out_file, dpi=300)
    print(f"Saved: {out_file}")
    if show:
        plt.show()
    plt.close(fig)
    return fig

def plot_successive_latency_percentiles(data_dir=None, output_dir=None, show=False):
    """
    Plots a grouped bar chart comparing p50, p95, and p99 for successive allocations across subjects.
    """
    data_path, out_path = resolve_paths(data_dir, output_dir)
    csv_file = data_path / "alloc_latency_successive.csv"
    out_file = out_path / "alloc_latency_successive_percentiles.png"

    if not csv_file.exists():
        print(f"Warning: {csv_file} not found. Skipping plot.")
        return None

    df = pd.read_csv(csv_file)
    pcts = df.groupby('subject')['latency'].quantile([0.5, 0.95, 0.99]).unstack(level=-1).reset_index()
    pcts.columns = ['subject', 'p50', 'p95', 'p99']

    sub_order = ['malloc', 'first_fit', 'best_fit', 'next_fit']
    pcts['order'] = pcts['subject'].map(lambda x: sub_order.index(x) if x in sub_order else 99)
    pcts = pcts.sort_values('order').reset_index(drop=True)

    fig, ax = plt.subplots(figsize=(9, 5), dpi=150)
    x = np.arange(len(pcts))
    width = 0.25

    ax.bar(x - width, pcts['p50'], width, label='p50 (Median)', color=PERCENTILE_COLORS['p50'], edgecolor='black', linewidth=0.8, alpha=0.85)
    ax.bar(x, pcts['p95'], width, label='p95 (95th %)', color=PERCENTILE_COLORS['p95'], edgecolor='black', linewidth=0.8, alpha=0.85)
    ax.bar(x + width, pcts['p99'], width, label='p99 (99th %)', color=PERCENTILE_COLORS['p99'], edgecolor='black', linewidth=0.8, alpha=0.85)

    ax.set_ylabel("Latency (μs)", fontsize=12, fontweight='bold')
    ax.set_title("Successive Allocation Latency: Percentiles (p50, p95, p99)", fontsize=14, fontweight='bold', pad=12)
    ax.set_xticks(x)
    ax.set_xticklabels([get_label(s) for s in pcts['subject']], fontsize=11, fontweight='bold')
    ax.grid(True, linestyle='--', axis='y', alpha=0.6)
    ax.legend(frameon=True, fontsize=10, loc='upper left')

    plt.tight_layout()
    fig.savefig(out_file, dpi=300)
    print(f"Saved: {out_file}")
    if show:
        plt.show()
    plt.close(fig)
    return fig

# =============================================================================
# 4) Fragmentation Experiment: External Fragmentation
# =============================================================================

def plot_external_fragmentation(data_dir=None, output_dir=None, show=False):
    """
    Plots External Fragmentation vs Number of Requests.
    External Fragmentation = 1 - (largest_free_block / mem_size_free)
    """
    data_path, out_path = resolve_paths(data_dir, output_dir)
    csv_file = data_path / "fragmentation.csv"
    out_file = out_path / "external_fragmentation.png"

    if not csv_file.exists():
        print(f"Warning: {csv_file} not found. Skipping plot.")
        return None

    df = pd.read_csv(csv_file)

    fig, ax = plt.subplots(figsize=(9, 5), dpi=150)

    subjects = [s for s in ['first_fit', 'best_fit', 'next_fit'] if s in df['subject'].unique()]

    for subject in subjects:
        sub_df = df[df['subject'] == subject].copy().reset_index(drop=True)
        sub_df['request_number'] = sub_df.index + 1

        free_mem = sub_df['mem_size_free'].values
        largest_block = sub_df['largest_free_block'].values
        
        ext_frag = np.where(free_mem > 0, (1.0 - (largest_block / free_mem)) * 100.0, 0.0)
        ext_frag = np.clip(ext_frag, 0.0, 100.0)

        color = get_color(subject)
        label = get_label(subject)

        ax.plot(sub_df['request_number'], ext_frag, 
                linewidth=2, color=color, label=label)

    ax.set_xlabel("Number of Requests", fontsize=12, fontweight='bold')
    ax.set_ylabel("External Fragmentation (%)", fontsize=12, fontweight='bold')
    ax.set_title("External Fragmentation vs Number of Requests", fontsize=14, fontweight='bold', pad=12)
    ax.grid(True, linestyle='--', alpha=0.6)
    ax.legend(frameon=True, fontsize=10, loc='best')

    plt.tight_layout()
    fig.savefig(out_file, dpi=300)
    print(f"Saved: {out_file}")
    if show:
        plt.show()
    plt.close(fig)
    return fig

# =============================================================================
# Master Runner
# =============================================================================

def plot_all(data_dir=None, output_dir=None, show=False):
    """Generates all benchmark plots including means and percentiles."""
    print("Generating benchmark plots...")
    # Means
    plot_alloc_latency(data_dir, output_dir, show=show)
    plot_dealloc_latency(data_dir, output_dir, show=show)
    plot_latency_successive(data_dir, output_dir, show=show)
    
    # Percentiles (p50, p95, p99)
    plot_alloc_latency_percentiles(data_dir, output_dir, show=show)
    plot_dealloc_latency_percentiles(data_dir, output_dir, show=show)
    plot_successive_latency_percentiles(data_dir, output_dir, show=show)
    
    # Fragmentation
    plot_external_fragmentation(data_dir, output_dir, show=show)
    print("All plots generated successfully!")

def main():
    parser = argparse.ArgumentParser(description="Plot memcpp benchmark results from CSVs.")
    parser.add_argument("--data-dir", default=None, help="Directory containing CSV files (default: bench/data)")
    parser.add_argument("--output-dir", default=None, help="Directory to save plot images (default: bench/plots)")
    parser.add_argument("--show", action="store_true", help="Display interactive plot windows")
    parser.add_argument("-e", "--experiment", 
                        choices=["all", "alloc", "dealloc", "successive", "ext_frag", "frag", "percentiles", "p50", "p95", "p99"],
                        default="all", help="Which experiment to plot (default: all)")
    args = parser.parse_args()

    if args.experiment == "all":
        plot_all(args.data_dir, args.output_dir, show=args.show)
    elif args.experiment == "percentiles":
        plot_alloc_latency_percentiles(args.data_dir, args.output_dir, show=args.show)
        plot_dealloc_latency_percentiles(args.data_dir, args.output_dir, show=args.show)
        plot_successive_latency_percentiles(args.data_dir, args.output_dir, show=args.show)
    elif args.experiment in ["p50", "p95", "p99"]:
        data_path, out_path = resolve_paths(args.data_dir, args.output_dir)
        pct = args.experiment
        plot_latency_vs_request_size(data_path / "alloc_latency.csv", f"Allocation Latency vs Request Size ({pct.upper()})", out_path / f"alloc_latency_{pct}.png", metric=pct, show=args.show)
        plot_latency_vs_request_size(data_path / "dealloc_latency.csv", f"Deallocation Latency vs Request Size ({pct.upper()})", out_path / f"dealloc_latency_{pct}.png", metric=pct, show=args.show)
    elif args.experiment == "alloc":
        plot_alloc_latency(args.data_dir, args.output_dir, show=args.show)
        plot_alloc_latency_percentiles(args.data_dir, args.output_dir, show=args.show)
    elif args.experiment == "dealloc":
        plot_dealloc_latency(args.data_dir, args.output_dir, show=args.show)
        plot_dealloc_latency_percentiles(args.data_dir, args.output_dir, show=args.show)
    elif args.experiment == "successive":
        plot_latency_successive(args.data_dir, args.output_dir, show=args.show)
        plot_successive_latency_percentiles(args.data_dir, args.output_dir, show=args.show)
    elif args.experiment in ["ext_frag", "frag"]:
        plot_external_fragmentation(args.data_dir, args.output_dir, show=args.show)

if __name__ == "__main__":
    main()
