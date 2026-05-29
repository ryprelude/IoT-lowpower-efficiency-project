import csv
from pathlib import Path

import matplotlib.pyplot as plt


ROOT = Path(__file__).resolve().parents[1]
DATA_FILES = [
    ROOT / "data" / "crypto_result_ASCON.csv",
    ROOT / "data" / "crypto_result_AED.csv",
]
RESULTS = ROOT / "results"


def load_rows():
    rows = []
    for path in DATA_FILES:
        with path.open(newline="", encoding="utf-8-sig") as f:
            for row in csv.DictReader(f):
                row = {key.strip(): value.strip() for key, value in row.items()}
                rows.append(
                    {
                        "algorithm": row["algorithm"].strip(),
                        "data_size": int(row["data_size"]),
                        "cpu_ticks": float(row["cpu_ticks"]),
                        "energy_mJ": float(row["energy_mJ"]),
                    }
                )
    return rows


def by_algorithm(rows):
    grouped = {}
    for row in rows:
        grouped.setdefault(row["algorithm"], []).append(row)
    for values in grouped.values():
        values.sort(key=lambda item: item["data_size"])
    return grouped


def save_energy_bar(grouped):
    sizes = [16, 32, 64]
    x = range(len(sizes))
    width = 0.34
    fig, ax = plt.subplots(figsize=(8, 5), dpi=160)
    colors = {"ASCON": "#2b8a6e", "AES-GCM": "#c65146"}
    offsets = {"ASCON": -width / 2, "AES-GCM": width / 2}
    labels = {"ASCON": "ASCON", "AES-GCM": "AES-GCM"}

    for alg in ["ASCON", "AES-GCM"]:
        values = {row["data_size"]: row["energy_mJ"] for row in grouped[alg]}
        ax.bar(
            [i + offsets[alg] for i in x],
            [values[size] for size in sizes],
            width,
            label=labels[alg],
            color=colors[alg],
        )

    ax.set_title("Energy Consumption by Payload Size")
    ax.set_xlabel("Payload size (bytes)")
    ax.set_ylabel("Energy (mJ)")
    ax.set_xticks(list(x), [str(size) for size in sizes])
    ax.grid(axis="y", alpha=0.25)
    ax.legend()
    ax.text(
        0.5,
        -0.20,
        "32B values: Cooja host-timing calibrated, not direct energy-meter readings",
        ha="center",
        transform=ax.transAxes,
        fontsize=8,
    )
    fig.tight_layout()
    fig.savefig(RESULTS / "energy_bar.png")
    plt.close(fig)


def save_comparison(grouped):
    fig, ax = plt.subplots(figsize=(8, 5), dpi=160)
    styles = {
        "ASCON": {"color": "#2b8a6e", "marker": "o", "label": "ASCON"},
        "AES-GCM": {"color": "#c65146", "marker": "s", "label": "AES-GCM"},
    }

    for alg in ["ASCON", "AES-GCM"]:
        rows = grouped[alg]
        ax.plot(
            [row["data_size"] for row in rows],
            [row["energy_mJ"] for row in rows],
            linewidth=2,
            **styles[alg],
        )

    ax.scatter([32], [grouped["ASCON"][1]["energy_mJ"]], s=95, facecolors="none", edgecolors="#2b8a6e")
    ax.scatter([32], [grouped["AES-GCM"][1]["energy_mJ"]], s=95, facecolors="none", edgecolors="#c65146")
    ax.set_title("Energy Consumption Trend")
    ax.set_xlabel("Payload size (bytes)")
    ax.set_ylabel("Energy (mJ)")
    ax.set_xticks([16, 32, 64])
    ax.grid(alpha=0.25)
    ax.legend()
    ax.text(
        0.5,
        -0.20,
        "Open 32B markers indicate host-timing calibrated values.",
        ha="center",
        transform=ax.transAxes,
        fontsize=8,
    )
    fig.tight_layout()
    fig.savefig(RESULTS / "comparison.png")
    plt.close(fig)


def save_avg_std(grouped):
    fig, ax = plt.subplots(figsize=(7, 5), dpi=160)
    algs = ["ASCON", "AES-GCM"]
    averages = [
        sum(row["energy_mJ"] for row in grouped[alg]) / len(grouped[alg])
        for alg in algs
    ]
    ax.bar(algs, averages, color=["#2b8a6e", "#c65146"], width=0.48)
    ax.set_title("Average Energy Across Payload Sizes")
    ax.set_ylabel("Energy (mJ)")
    ax.grid(axis="y", alpha=0.25)
    ax.text(
        0.5,
        -0.16,
        "Average includes 16B, 32B, and 64B values.",
        ha="center",
        transform=ax.transAxes,
        fontsize=8,
    )
    fig.tight_layout()
    fig.savefig(RESULTS / "avg_std.png")
    plt.close(fig)


def main():
    RESULTS.mkdir(exist_ok=True)
    grouped = by_algorithm(load_rows())
    save_energy_bar(grouped)
    save_comparison(grouped)
    save_avg_std(grouped)


if __name__ == "__main__":
    main()
