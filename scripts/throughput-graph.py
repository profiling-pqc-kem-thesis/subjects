import sys
import sqlite3

import matplotlib
import matplotlib.ticker as ticker
import matplotlib.pyplot as plot
from pathlib import Path


def get_data(cursor: sqlite3.Cursor, algorithm_name: str, parameters: str, compiler: str, feature: str, stage: str):
    cursor.execute('''
        SELECT
            stage, numberOfThreads, throughput
        FROM
            algorithm
            INNER JOIN benchmark ON benchmark.algorithm = algorithm.id
            INNER JOIN parallelBenchmark ON parallelBenchmark.benchmark = benchmark.id
        WHERE
            algorithm.name = ? AND
            algorithm.parameters = ? AND
            algorithm.compiler = ? AND
            algorithm.features = ? AND
            benchmark.stage = ?
        ORDER BY parallelBenchmark.numberOfThreads
    ''', (algorithm_name, parameters, compiler, feature, stage,))

    return cursor.fetchall()


def draw(path: Path, algorithm_name: str, parameters: str):
    connection = sqlite3.connect(path)
    cursor = connection.cursor()

    compilers = ["gcc", "clang"]
    stages = ["keypair", "decrypt", "encrypt"]

    matplotlib.rcParams.update({'figure.autolayout': True})
    figure, axes_list = plot.subplots(1, 3)

    for i, stage in enumerate(stages):
        axes = axes_list[i]
        for compiler in compilers:
            data = get_data(cursor, algorithm_name, parameters, compiler, "avx2-optimized", stage)
            yy = [row[2] for row in data]
            axes.plot(range(1, len(yy) + 1), yy, marker='o')

        labels = [pow(2, i) for i in range(0, len(yy))]
        axes.xaxis.set_major_locator(ticker.FixedLocator(range(1, len(labels) + 1)))
        axes.set_xticklabels(labels=labels, fontsize=7)
        for tick in axes.yaxis.get_major_ticks():
            tick.label.set_fontsize(7)
        axes.set_title(stage, fontsize=8)

    axes_list[0].set_ylabel("Throughput")
    axes_list[1].set_xlabel("Number of threads")
    axes_list[2].legend(compilers, title="Compilers", bbox_to_anchor=(1.05, 1), loc='upper left', fontsize=8)
    axes_list[2].get_legend().get_title().set_fontsize('8')


    return figure


def main(path: Path):
    ntru_parameters = ["hps4096821", "hrss701"]
    mceliece_parameters = ["6960119f", "8192128f"]
    for parameter in ntru_parameters:
        title = "NTRU" + " " + parameter
        figure = draw(path, "ntru", parameter)
        figure.suptitle(title, fontsize=10)
        figure.savefig(title + ".pdf")

    for parameter in mceliece_parameters:
        title = "McEliece" + " " + parameter
        figure = draw(path, "mceliece", parameter)
        figure.suptitle(title, fontsize=10)
        figure.savefig(title + ".pdf")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("usage: {} <database_path>".format(sys.argv[0]))
        exit(1)
    main(Path(sys.argv[1]))
