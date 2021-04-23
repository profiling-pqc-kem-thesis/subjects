import os
import sqlite3
import sys

import numpy as np
import matplotlib.pyplot as plot
import matplotlib.ticker as ticker
from pathlib import Path
from typing import List

import pandas as pd


def get_data(cursor: sqlite3.Cursor, algorithm_name: str, parameters: str, event: str, compiler: str, feature: str, environment_name: str):
    cursor.execute('''
    SELECT
        benchmark.stage, microBenchmarkMeasurement.region, AVG(microBenchmarkEvent.value)
    FROM
        algorithm
        INNER JOIN benchmark ON benchmark.algorithm = algorithm.id
        INNER JOIN benchmarkRun ON benchmarkRun.id = benchmark.benchmarkRun
        INNER JOIN environment ON environment.id = benchmarkRun.environment
        INNER JOIN microBenchmark ON microBenchmark.benchmark = benchmark.id
        INNER JOIN microBenchmarkMeasurement ON microBenchmarkMeasurement.microBenchmark = microBenchmark.id
        INNER JOIN microBenchmarkEvent ON microBenchmarkEvent.microBenchmarkMeasurement = microBenchmarkMeasurement.id
    WHERE
        algorithm.name = ? AND
        algorithm.parameters = ? AND
        algorithm.compiler = ? AND
        algorithm.features = ? AND
        microBenchmarkEvent.event = ? AND
        environment.name = ? AND
        microBenchmarkEvent.value >= 0
    GROUP BY benchmark.stage, microBenchmarkMeasurement.region
    ''', (algorithm_name, parameters, compiler, feature, event, environment_name, ))

    return cursor.fetchall()


def get_distinct_regions(cursor: sqlite3.Cursor, algorithm_name: str, parameters: str, event: str):
    cursor.execute('''
    SELECT
        DISTINCT microBenchmarkMeasurement.region
    FROM
        algorithm
        INNER JOIN benchmark ON benchmark.algorithm = algorithm.id
        INNER JOIN benchmarkRun ON benchmarkRun.id = benchmark.benchmarkRun
        INNER JOIN microBenchmark ON microBenchmark.benchmark = benchmark.id
        INNER JOIN microBenchmarkMeasurement ON microBenchmarkMeasurement.microBenchmark = microBenchmark.id
        INNER JOIN microBenchmarkEvent ON microBenchmarkEvent.microBenchmarkMeasurement = microBenchmarkMeasurement.id
    WHERE
        algorithm.name = ? AND
        algorithm.parameters = ? AND
        microBenchmarkEvent.event = ? AND
        microBenchmarkMeasurement.region != "crypto_kem_keypair" AND
        microBenchmarkMeasurement.region != "crypto_kem_enc"  AND
        microBenchmarkMeasurement.region != "crypto_kem_dec" 
        
    ''', (algorithm_name, parameters, event,))
    return [item[0] for item in cursor.fetchall()]


def create_data_frames(path: Path, algorithm_name: str, parameters: str, event: str, environment_name: str):
    connection = sqlite3.connect(path)
    cursor = connection.cursor()
    compilers = ["gcc", "clang"]
    features = ["ref", "ref-optimized", "avx2", "avx2-optimized"]
    regions = get_distinct_regions(cursor, algorithm_name, parameters, "cpu-cycles")
    regions.append("other")
    print(regions)

    index = []
    keypair_data = []
    encrypt_data = []
    decrypt_data = []

    def parse_data(data: List):
        print(data)
        other_index = regions.index("other")
        temp_keypair_data = np.zeros(shape=(len(regions)))
        temp_encrypt_data = np.zeros(shape=(len(regions)))
        temp_decrypt_data = np.zeros(shape=(len(regions)))
        for row in data:
            if row[1] == "crypto_kem_keypair":
                temp_keypair_data[other_index] = row[2]
            elif row[1] == "crypto_kem_enc":
                temp_encrypt_data[other_index] = row[2]
            elif row[1] == "crypto_kem_dec":
                temp_decrypt_data[other_index] = row[2]
            elif row[0] == "keypair":
                temp_keypair_data[regions.index(row[1])] = row[2]
            elif row[0] == "encrypt":
                temp_encrypt_data[regions.index(row[1])] = row[2]
            elif row[0] == "decrypt":
                temp_decrypt_data[regions.index(row[1])] = row[2]

        temp_keypair_data[other_index] = (2 * temp_keypair_data[other_index]) - sum(temp_keypair_data.tolist())
        temp_encrypt_data[other_index] = (2 * temp_encrypt_data[other_index]) - sum(temp_encrypt_data.tolist())
        temp_decrypt_data[other_index] = (2 * temp_decrypt_data[other_index]) - sum(temp_decrypt_data.tolist())

        keypair_data.append(temp_keypair_data)
        encrypt_data.append(temp_encrypt_data)
        decrypt_data.append(temp_decrypt_data)

    for feature in features:
        if feature.endswith("-optimized"):
            for compiler in compilers:
                index.append(compiler + " " + feature)
                parse_data(get_data(cursor, algorithm_name, parameters, event, compiler, feature, environment_name))
        else:
            index.append("gcc " + feature)
            parse_data(get_data(cursor, algorithm_name, parameters, event, "gcc", feature, environment_name))

    connection.close()

    keypair = pd.DataFrame(keypair_data, index=index, columns=regions)
    encrypt = pd.DataFrame(encrypt_data, index=index, columns=regions)
    decrypt = pd.DataFrame(decrypt_data, index=index, columns=regions)
    return keypair, encrypt, decrypt, index


def plot_clustered_stacked(dataframes, names=None, title="multiple stacked bar plot", H="/", **kwargs):
    """Given a list of dataframes, with identical columns and index, create a clustered stacked bar plot.
labels is a list of the names of the dataframe, used for the legend
title is a string for the title of the plot
H is the hatch used for identification of the different dataframe"""

    n_df = len(dataframes)
    n_col = len(dataframes[0].columns)
    n_ind = len(dataframes[0].index)

    rectangle_width = 1 / float(n_df + 1.5)

    axes = plot.subplot(111)

    for frame in dataframes:
        axes = frame.plot(kind="bar", linewidth=0, stacked=True, ax=axes, legend=False, grid=False, **kwargs)

    pos = []
    handles, labels = axes.get_legend_handles_labels()
    for i in range(0, n_df * n_col, n_col):  # len(h) = n_col * n_df
        for j, handle in enumerate(handles[i:i + n_col]):
            for rectangle in handle.patches:
                rectangle.set_x(rectangle.get_x() + 1 / float(n_df + 1) * i / float(n_col))
                rectangle.set_width(rectangle_width)
                pos.append(rectangle.get_x() + (rectangle_width / 2))

    pos = sorted(list(set(pos)))
    print(len(pos))
    axes.xaxis.set_major_locator(ticker.FixedLocator(pos))
    axes.set_xticklabels(labels=["key-pair", "encrypt", "decrypt"] * n_ind, rotation=-60, fontsize=8)
    axes.set_title(title)

    print(n_ind)
    # Draw the zones for the different optimizations
    for i in range(n_ind):
        plot.axvspan(pos[i * 3] - rectangle_width, pos[i * 3 + 2] + rectangle_width, facecolor="#7085c4", alpha=0.2,
                     zorder=-100)
        axes.text(pos[i * 3] - rectangle_width, 3.5, names[i], {'ha': 'left', 'va': 'bottom'}, rotation=90, fontsize=6)

    plot.legend(reversed(handles[:n_col]), reversed(labels[:n_col]), loc='upper left', bbox_to_anchor=(1.05, 1))

    return axes


def main(path: Path):
    environment_names = ["Modern Workstation", "Modern Laptop", "Old Mid-Range Laptop", "Old Low-Range Laptop",
                         "Cloud Provider 1", "Cloud Provider 2", "IBM Community Cloud"]
    ntru_parameters = ["hps4096821", "hrss701"]
    mceliece_parameters = ["6960119", "6960119f", "8192128", "8192128f"]
    event = "task-clock"

    os.makedirs("build", exist_ok=True)

    for environment_name in environment_names:
        os.makedirs("build/" + environment_name, exist_ok=True)
        for parameter in ntru_parameters:
            keypair, encrypt, decrypt, index = create_data_frames(path, "ntru", parameter, event, environment_name)
            title = "NTRU " + parameter + " - " + event
            plot_clustered_stacked([keypair, encrypt, decrypt], names=index, title=title)
            plot.savefig("build/" + environment_name + "/" + title + ".pdf", bbox_inches='tight')
            plot.close()

        for parameter in mceliece_parameters:
            keypair, encrypt, decrypt, index = create_data_frames(path, "mceliece", parameter, event, environment_name)
            title = "McEliece " + parameter + " - " + event
            plot_clustered_stacked([keypair, encrypt, decrypt], names=index, title=title)
            plot.savefig("build/" + environment_name + "/" + title + ".pdf", bbox_inches='tight')
            plot.close()


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("usage: {} <database_path>".format(sys.argv[0]))
        exit(1)
    main(Path(sys.argv[1]))
