import re
import sqlite3
import sys
from typing import TextIO
from datetime import datetime
from os import listdir
from pathlib import Path
from shutil import copyfile


class Parse:

    def __init__(self, path: Path):
        self.path = path
        self.copy_environment_database()
        self.connection = sqlite3.connect("data.db")
        self.cursor = self.connection.cursor()
        self.init_database()

        self.algorithms = {}
        self.parse_algorithms()

    def __del__(self):
        self.connection.commit()
        self.connection.close()

    def copy_environment_database(self):
        copyfile(self.path.joinpath("environment.sqlite"), "data.db")

    def init_database(self):

        #    cursor.execute('''CREATE TABLE IF NOT EXISTS environment
        #               (name TEXT PRIMARY KEY, date TEXT, uname TEXT, cores INTEGER, threads INTEGER, memory INTEGER,
        #                memory_speed TEXT, cpu TEXT, cpu_features TEXT)''')
        self.cursor.execute('''CREATE TABLE IF NOT EXISTS benchmark
                   (id INTEGER PRIMARY KEY, environment TEXT, benchmark_type TEXT, stage TEXT, algorithm INTEGER,
                    totalDuration INTEGER, startTimestamp INTEGER, stopTimestamp INTEGER)''')
        self.cursor.execute('''CREATE TABLE IF NOT EXISTS algorithm
                   (id INTEGER PRIMARY KEY, name TEXT, parameters TEXT, compiler TEXT, features TEXT)''')
        # Sequential benchmark
        self.cursor.execute('''CREATE TABLE IF NOT EXISTS sequentialBenchmark
                   (id INTEGER PRIMARY KEY, benchmark INTEGER, iterations INTEGER, averageDuration REAL)''')
        self.cursor.execute('''CREATE TABLE IF NOT EXISTS sequentialBenchmarkIteration
                   (id INTEGER PRIMARY KEY, sequentialBenchmark INTEGER, iteration INTEGER,
                    duration UNSIGNED BIG INT)''')
        # Parallel benchmark
        self.cursor.execute('''CREATE TABLE IF NOT EXISTS parallelBenchmark
                   (id INTEGER PRIMARY KEY, benchmark INTEGER, numberOfThreads INTEGER, averageDurationPerIteration REAL,
                    throughput REAL, totalIterations REAL, averageIterationsPerThread INTEGER)''')
        self.cursor.execute('''CREATE TABLE IF NOT EXISTS parallelBenchmarkThread
                   (id INTEGER PRIMARY KEY, parallelBenchmark INTEGER, thread INTEGER, iterations INTEGER, duration UNSIGNED BIG INT)''')
        # Micro benchmark
        self.cursor.execute('''CREATE TABLE IF NOT EXISTS microBenchmark (id INTEGER PRIMARY KEY, benchmark INTEGER)''')
        self.cursor.execute('''CREATE TABLE IF NOT EXISTS microBenchmarkMeasurement
                   (id INTEGER PRIMARY KEY, microBenchmark INTEGER, region TEXT)''')
        self.cursor.execute('''CREATE TABLE IF NOT EXISTS microBenchmarkEvent
                   (id INTEGER PRIMARY KEY, microBenchmarkMeasurement INTEGER, event TEXT, value UNSIGNED BIG INT)''')

    def parse_algorithms(self):
        file_names = [file_name.split(".")[0] for file_name in listdir(self.path.joinpath("sequential"))]
        for file_name in set(file_names):
            parts = file_name.split("_")
            if parts[0] == "dh":
                self.cursor.execute(
                    "INSERT INTO algorithm(name, parameters, compiler, features) VALUES (?, ?, ?, ?)",
                    ("dh", "", parts[1], parts[2]))
            else:
                self.cursor.execute(
                    "INSERT INTO algorithm(name, parameters, compiler, features) VALUES (?, ?, ?, ?)",
                    (parts[0], parts[1], parts[2], parts[3]))
            self.algorithms[file_name] = self.cursor.lastrowid

    def add_benchmark(self, benchmark_type: str, file_name: str, start_timestamp: int, stop_timestamp: int) -> int:
        parts = file_name.split(".")
        self.cursor.execute(
            "INSERT INTO benchmark(environment, benchmark_type, stage, algorithm, totalDuration, startTimestamp, stopTimestamp) VALUES (?, ?, ?, ?, ?, ?, ?)",
            (self.path.name, benchmark_type, parts[1], self.algorithms[parts[0]], stop_timestamp - start_timestamp, start_timestamp, stop_timestamp))
        return self.cursor.lastrowid

    def get_times(self, file: TextIO) -> (int, int):
        time_pattern = re.compile("[0-9\-]{10} [0-9\:]{8}")

        lines = file.read().splitlines()
        start_time = re.search(time_pattern, lines[0]).group(0)
        start_time = datetime.strptime(start_time, "%Y-%m-%d %H:%M:%S") - datetime(1900, 1, 1)
        start_time = int(start_time.total_seconds())

        stop_time = re.search(time_pattern, lines[-1]).group(0)
        stop_time = datetime.strptime(stop_time, "%Y-%m-%d %H:%M:%S") - datetime(1900, 1, 1)
        stop_time = int(stop_time.total_seconds())
        file.seek(0)

        return start_time, stop_time

    def sequential(self):
        sequential_path = self.path.joinpath("sequential")

        average_pattern = re.compile("average \(of [0-9]+ iterations\): [0-9\.]+ms")
        end_pattern = re.compile("=== [0-9\-]{10} [0-9\:]{8} Command ended with exit code 0 ===")

        for file_name in listdir(sequential_path):
            with iter(open(sequential_path.joinpath(file_name))) as input_file:
                sequential_benchmark_iterations = []
                sequential_benchmark_id = None
                start_time, stop_time = self.get_times(input_file)
                benchmark_id = self.add_benchmark("sequential", file_name, start_time, stop_time)

                results = False
                summary = False
                for line in input_file:
                    if line.strip() == "iteration,duration (ns)":
                        results = True
                    elif line.strip() == "=== Summary ===":
                        results = False
                        summary = True
                    elif results:
                        sequential_benchmark_iterations.append(tuple(line.split(",")))
                    elif summary:
                        line = next(input_file)
                        res = re.search(average_pattern, line)
                        iterations = int(re.search("of [0-9]+ iterations", res.group(0)).group(0).split(" ")[1])
                        average_duration = float(re.search("[0-9\.]+ms", res.group(0)).group(0)[:-2])
                        self.cursor.execute(
                            "INSERT INTO sequentialBenchmark(benchmark, iterations, averageDuration) VALUES (?, ?, ?)",
                            (benchmark_id, iterations, average_duration))
                        sequential_benchmark_id = self.cursor.lastrowid
                        break

                sequential_benchmark_iterations = [(sequential_benchmark_id,) + benchmark for benchmark in sequential_benchmark_iterations]
                self.cursor.executemany(
                    "INSERT INTO sequentialBenchmarkIteration(sequentialBenchmark, iteration, duration) VALUES (?, ?, ?)",
                    sequential_benchmark_iterations)

    def parallel(self):
        parallel_path = self.path.joinpath("parallel")
        end_pattern = re.compile("=== [0-9\-]{10} [0-9\:]{8} Command ended with exit code 0 ===")

        for file_name in listdir(parallel_path):
            with iter(open(parallel_path.joinpath(file_name))) as input_file:
                parallel_benchmark_threads = []
                parallel_benchmark_id = None
                start_time, stop_time = self.get_times(input_file)
                benchmark_id = self.add_benchmark("parallel", file_name, start_time, stop_time)

                results = False
                summary = False
                for line in input_file:
                    if line.strip() == "thread,iterations,duration (ns)":
                        results = True
                    elif line.strip() == "=== Summary ===":
                        results = False
                        summary = True
                    elif results:
                        parallel_benchmark_threads.append(tuple(line.split(",")))
                    elif summary:
                        total_iterations = int(re.search("[0-9]+ total", line).group(0)[:-6])
                        number_of_threads = int(re.search("[0-9]+ threads", line).group(0)[:-8])
                        average_iterations_per_thread = float(re.search(": [0-9\.]+", line).group(0)[2:])
                        line = next(input_file)
                        average_duration_per_iteration = float(re.search("[0-9\.]+", line).group(0))
                        next(input_file)
                        line = next(input_file)
                        throughput = float(re.search("[0-9\.]+", line).group(0))
                        self.cursor.execute(
                            "INSERT INTO parallelBenchmark(benchmark, numberOfThreads, averageDurationPerIteration, throughput, totalIterations, averageIterationsPerThread) VALUES (?, ?, ?, ?, ? ,?)",
                            (benchmark_id, number_of_threads, average_duration_per_iteration, throughput, total_iterations,
                             average_iterations_per_thread))
                        parallel_benchmark_id = self.cursor.lastrowid
                        break

                parallel_benchmark_threads = [(parallel_benchmark_id,) + benchmark for benchmark in parallel_benchmark_threads]
                self.cursor.executemany(
                    "INSERT INTO parallelBenchmarkThread(parallelBenchmark, thread, iterations, duration) VALUES (?, ?, ?, ?)",
                    parallel_benchmark_threads)

    def micro(self):
        micro_path = self.path.joinpath("micro")
        end_pattern = re.compile("=== [0-9\-]{10} [0-9\:]{8} Command ended with exit code 0 ===")

        for file_name in listdir(micro_path):
            with iter(open(micro_path.joinpath(file_name))) as input_file:
                parallel_benchmark_threads = []
                parallel_benchmark_id = None
                start_time, stop_time = self.get_times(input_file)
                benchmark_id = self.add_benchmark("micro", file_name, start_time, stop_time)
                self.cursor.execute("INSERT INTO microBenchmark(benchmark) VALUES (?)", (benchmark_id, ))
                micro_benchmark_id = self.cursor.lastrowid

                csv_header = None
                result = False
                for line in input_file:
                    if re.search("INFO: [0-9]+: exited", line):
                        line = next(input_file)
                        csv_header = line.split(",")
                        result = True
                    elif re.search(end_pattern, line) is not None:
                        result = False
                    elif result:
                        for i, item in enumerate(line.split(",")[:-1]):
                            if i == 0:
                                self.cursor.execute("INSERT INTO MicroBenchmarkMeasurement(microBenchmark, region) VALUES (?, ?)",
                                                    (micro_benchmark_id, item))
                                micro_benchmark_measurement_id = self.cursor.lastrowid
                            else:
                                self.cursor.execute("INSERT INTO MicroBenchmarkEvent(microBenchmarkMeasurement, event, value) VALUES (?, ?, ?)",
                                                    (micro_benchmark_measurement_id, csv_header[i], item))


def main():
    parse = Parse(Path(sys.argv[1]))
    parse.sequential()
    parse.parallel()
    parse.micro()
    del parse



if __name__ == "__main__":
    main()
