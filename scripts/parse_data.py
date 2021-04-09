import re
import sqlite3
import sys
from os import listdir
from pathlib import Path
from shutil import copyfile

connection = None
cursor = None


def init_data_base():
    global connection
    connection = sqlite3.connect("data.db")
    global cursor
    cursor = connection.cursor()
#    cursor.execute('''CREATE TABLE IF NOT EXISTS environment
#               (name TEXT PRIMARY KEY, date TEXT, uname TEXT, cores INTEGER, threads INTEGER, memory INTEGER,
#                memory_speed TEXT, cpu TEXT, cpu_features TEXT)''')
    cursor.execute('''CREATE TABLE IF NOT EXISTS benchmark
               (id INTEGER PRIMARY KEY, environment INTEGER, type TEXT, algorithm INTEGER,
                benchmark INTEGER, totalDuration INTEGER, startTimestamp INTEGER, stopTimestamp INTEGER)''')
    cursor.execute('''CREATE TABLE IF NOT EXISTS algorithm
               (id INTEGER PRIMARY KEY, name TEXT, parameters TEXT, compiler TEXT, features TEXT)''')
    cursor.execute('''CREATE TABLE IF NOT EXISTS sequentialBenchmark
               (id INTEGER PRIMARY KEY, benchmark INTEGER, iterations INTEGER, averageDuration REAL)''')
    cursor.execute('''CREATE TABLE IF NOT EXISTS sequentialBenchmarkIteration
               (id INTEGER PRIMARY KEY, sequentialBenchmark INTEGER, iteration INTEGER,
                duration UNSIGNED BIG INT)''')


def copy_environment_data_base(path: Path):
    copyfile(path.joinpath("environment.sqlite"), "data.db")


def parse_algorithm(file_name: str) -> int:
    parts = file_name.split("_")
    if parts[0] == "dh":
        cursor.execute(
            "INSERT INTO algorithm(name, parameters, compiler, features) VALUES (?, ?, ?, ?)",
            ("dh", "", parts[1], parts[2].split(".")[0]))
    else:
        cursor.execute(
            "INSERT INTO algorithm(name, parameters, compiler, features) VALUES (?, ?, ?, ?)",
            (parts[0], parts[1], parts[2], parts[3].split(".")[0]))
    return cursor.lastrowid


def parse_benchmark(environment: int, benchmark_type: str, algorithm: int, benchmark: int, total_duration: int, start_timestamp: int, stop_timestamp: int):
    cursor.execute(
        "INSERT INTO benchmark(environment, type, algorithm, benchmark, totalDuration, startTimestamp, stopTimestamp) VALUES (?, ?, ?, ?, ?, ?, ?)",
        (environment, benchmark_type, algorithm, benchmark, total_duration, start_timestamp, stop_timestamp))


def parse_sequential(path: Path):
    sequential_path = path.joinpath("sequential")

    average_pattern = re.compile("average \(of [0-9]+ iterations\): [0-9\.]+ms")
    end_pattern = re.compile("=== [0-9\-]{10} [0-9\:]{8} Command ended with exit code 0 ===")

    for file_name in listdir(sequential_path):
        algorithm_id = parse_algorithm(file_name)
        with open(sequential_path.joinpath(file_name)) as input_file:
            results = False
            summary = False
            for line in input_file:
                if line.strip() == "iteration,duration (ns)":
                    results = True
                    continue
                elif line.strip() == "=== Summary ===":
                    results = False
                    summary = True
                    continue
                elif re.search(end_pattern, line) is not None:
                    break
                elif results:
                    parts = line.split(",")
                    cursor.execute("INSERT INTO sequentialBenchmarkIteration(sequentialBenchmark, iteration, duration) VALUES (?, ?, ?)", (0, int(parts[0]), int(parts[1])))
                elif summary:
                    res = re.search(average_pattern, line)
                    if res is not None:
                        print(res.group(0))
                    #parts = line.split(",")
                    #cursor.execute("INSERT INTO sequentialBenchmark(benchmark, iterations, averageDuration) VALUES (?, ?, ?)", (0, int(parts[0]), int(parts[1])))

def main():
    path = Path(sys.argv[1])
    copy_environment_data_base(path)
    init_data_base()
    if connection is None and cursor is None:
        exit(1)

    parse_sequential(path)
    connection.commit()
    connection.close()


if __name__ == "__main__":
    main()
