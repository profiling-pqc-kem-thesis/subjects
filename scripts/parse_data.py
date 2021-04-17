import os
import re
import subprocess
import sqlite3
import sys

from datetime import datetime
from pathlib import Path
from shutil import copyfile
from tempfile import NamedTemporaryFile
from typing import List, TextIO, Tuple

MAX_INT = 9223372036854775807


class Parse:

    def __init__(self, path, database_path):
        self.path = path
        self.database_path = database_path

    def __del__(self):
        self.connection.close()

    def commit(self):
        self.connection.commit()

    def initialize(self):
        self.connection = sqlite3.connect(self.database_path)
        self.cursor = self.connection.cursor()
        self.init_database()

        self.algorithms = {}
        self.parse_algorithms()
        self.environment_id = self.environment()

    def rollback(self):
        if self.connection is not None:
            self.connection.rollback()

    def init_database(self):
        # Environment
        self.cursor.execute('''CREATE TABLE IF NOT EXISTS environment
                   (id INTEGER PRIMARY KEY, name TEXT UNIQUE, date TEXT, uname TEXT, cores INT, threads INT, memory INT,
                    memory_speed TEXT, cpu TEXT, cpu_features TEXT)''')
        # Tool versions
        self.cursor.execute('''CREATE TABLE IF NOT EXISTS tool
                   (id INTEGER PRIMARY KEY, environment INT, tool TEXT, version TEXT,
                   UNIQUE(environment, tool))''')
        # Benchmark
        self.cursor.execute('''CREATE TABLE IF NOT EXISTS benchmark
                   (id INTEGER PRIMARY KEY, environment INTEGER, benchmark_type TEXT, stage TEXT, algorithm INTEGER,
                    totalDuration INTEGER, startTimestamp INTEGER, stopTimestamp INTEGER)''')
        # Algorithm
        self.cursor.execute('''CREATE TABLE IF NOT EXISTS algorithm
                   (id INTEGER PRIMARY KEY, name TEXT, parameters TEXT, compiler TEXT, features TEXT,
                   UNIQUE(name, parameters, compiler, features))''')
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
        # Stack benchmark
        self.cursor.execute('''CREATE TABLE IF NOT EXISTS stackBenchmark (id INTEGER PRIMARY KEY, benchmark INTEGER)''')
        self.cursor.execute('''CREATE TABLE IF NOT EXISTS stackBenchmarkSymbol
                   (id INTEGER PRIMARY KEY, stackBenchmark INTEGER, symbol TEXT, size INTEGER, allocation INTEGER)''')
        # Heap benchmark
        self.cursor.execute('''CREATE TABLE IF NOT EXISTS heapBenchmark (id INTEGER PRIMARY KEY, benchmark INTEGER)''')
        self.cursor.execute('''CREATE TABLE IF NOT EXISTS heapBenchmarkMeasurement
                   (id INTEGER PRIMARY KEY, heapBenchmark INTEGER, trace TEXT, peakAllocation INTEGER)''')


    def parse_algorithms(self):
        file_names = [file_name.split(".")[0] for file_name in os.listdir(self.path.joinpath("stack"))]
        for file_name in set(file_names):
            parts = file_name.split("_")
            parameters = ("dh", "", parts[1], parts[2]) if parts[0] == "dh" else  (parts[0], parts[1], parts[2], parts[3])
            try:
                self.cursor.execute(
                    "INSERT INTO algorithm(name, parameters, compiler, features) VALUES (?, ?, ?, ?)",
                    parameters)
            except sqlite3.IntegrityError:
                pass
            finally:
                self.cursor.execute(
                    "SELECT id FROM algorithm WHERE name = ? AND parameters = ? AND compiler = ? AND features = ?",
                    parameters)
                (id,) = self.cursor.fetchone()
                self.algorithms[file_name] = id

    def add_benchmark(self, benchmark_type: str, file_name: str, start_timestamp: int, stop_timestamp: int) -> int:
        parts = file_name.split(".")
        stage = ""
        if parts[1] == "keypair" or parts[1] == "encrypt" or parts[1] == "decrypt" or parts[1] == "exchange":
            stage = parts[1]

        self.cursor.execute(
            "INSERT INTO benchmark(environment, benchmark_type, stage, algorithm, totalDuration, startTimestamp, stopTimestamp) VALUES (?, ?, ?, ?, ?, ?, ?)",
            (self.environment_id, benchmark_type, stage, self.algorithms[parts[0]], stop_timestamp - start_timestamp,
             start_timestamp, stop_timestamp))
        return self.cursor.lastrowid

    @staticmethod
    def was_skipped(file: TextIO) -> bool:
        head = file.readline()
        file.seek(0)
        return re.search("^warning: skipping", head) is not None

    @staticmethod
    def get_times(file: TextIO) -> (int, int):
        time_pattern = re.compile("[0-9-]{10} [0-9:]{8}")
        lines = file.read().splitlines()

        start_time = time_pattern.search(lines[0])
        if start_time is None:
            print("Warning: unable to get start time from file:", file.name)
            file.seek(0)
            return (0, 0)
        start_time = start_time.group(0)
        start_time = datetime.strptime(start_time, "%Y-%m-%d %H:%M:%S") - datetime(1970, 1, 1)
        start_time = int(start_time.total_seconds())

        stop_time = time_pattern.search(lines[-1])
        if stop_time is None:
            print("Warning: unable to get end time from file:", file.name)
            file.seek(0)
            return (0, 0)
        stop_time = stop_time.group(0)
        stop_time = datetime.strptime(stop_time, "%Y-%m-%d %H:%M:%S") - datetime(1970, 1, 1)
        stop_time = int(stop_time.total_seconds())
        file.seek(0)
        return start_time, stop_time

    def environment(self) -> int:
        environment_path = self.path.joinpath("environment.sqlite")
        environment_connection = sqlite3.connect(environment_path)
        environment_cursor = environment_connection.cursor()
        environment_cursor.execute("SELECT * FROM environments")
        result = environment_cursor.fetchone()
        self.cursor.execute(
            "INSERT INTO environment(name, date, uname, cores, threads, memory,  memory_speed, cpu, cpu_features) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)",
            result)
        environment_cursor.close()
        environment_connection.close()
        return self.cursor.lastrowid

    def tool_versions(self):
        versions_path = self.path.joinpath("versions")

        for file_name in os.listdir(versions_path):
            file_path = self.path.joinpath("versions/{}".format(file_name))
            tool = file_name.replace(".txt", "")
            with open(file_path, "r") as file:
                version = file.read()
                self.cursor.execute(
                    "INSERT OR IGNORE INTO tool(environment, tool, version) VALUES (?, ?, ?)",
                    (self.environment_id, tool, version)
                )

    def sequential(self):
        sequential_path = self.path.joinpath("sequential")
        average_pattern = re.compile("average \(of [0-9]+ iterations\): [0-9\.]+ms")

        for file_name in os.listdir(sequential_path):
            with iter(open(sequential_path.joinpath(file_name))) as input_file:
                if self.was_skipped(input_file):
                    continue

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

                sequential_benchmark_iterations = [(sequential_benchmark_id, int(benchmark[0]), int(benchmark[1])) for
                                                   benchmark in sequential_benchmark_iterations]
                self.cursor.executemany(
                    "INSERT INTO sequentialBenchmarkIteration(sequentialBenchmark, iteration, duration) VALUES (?, ?, ?)",
                    sequential_benchmark_iterations)


    def parallel(self):
        parallel_path = self.path.joinpath("parallel")

        for file_name in os.listdir(parallel_path):
            with iter(open(parallel_path.joinpath(file_name))) as input_file:
                if self.was_skipped(input_file):
                    continue

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
                            (benchmark_id, number_of_threads, average_duration_per_iteration, throughput,
                             total_iterations,
                             average_iterations_per_thread))
                        parallel_benchmark_id = self.cursor.lastrowid
                        break

                parallel_benchmark_threads = [
                    (parallel_benchmark_id, int(benchmark[0]), int(benchmark[1]), int(benchmark[2])) for benchmark in
                    parallel_benchmark_threads]
                self.cursor.executemany(
                    "INSERT INTO parallelBenchmarkThread(parallelBenchmark, thread, iterations, duration) VALUES (?, ?, ?, ?)",
                    parallel_benchmark_threads)


    def micro(self):
        micro_path = self.path.joinpath("micro")
        end_pattern = re.compile("=== [0-9\-]{10} [0-9\:]{8} Command ended with exit code 0 ===")

        for file_name in os.listdir(micro_path):
            with iter(open(micro_path.joinpath(file_name))) as input_file:
                if self.was_skipped(input_file):
                    continue

                start_time, stop_time = self.get_times(input_file)
                benchmark_id = self.add_benchmark("micro", file_name, start_time, stop_time)
                self.cursor.execute("INSERT INTO microBenchmark(benchmark) VALUES (?)", (benchmark_id,))
                micro_benchmark_id = self.cursor.lastrowid

                csv_header = None
                result = False
                found_rows_to_ignore = False
                micro_benchmark_measurement_id = None
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
                                if not found_rows_to_ignore:
                                    # Ignore all rows correlating to calls from the get_global_state function
                                    found_kem_end = item == "crypto_kem_enc"
                                    if found_kem_end:
                                        found_rows_to_ignore = True
                                else:
                                    self.cursor.execute(
                                        "INSERT INTO MicroBenchmarkMeasurement(microBenchmark, region) VALUES (?, ?)",
                                        (micro_benchmark_id, item))
                                    micro_benchmark_measurement_id = self.cursor.lastrowid
                            else:
                                item = int(item)
                                # Outlier - identified as bug in perf
                                if item == MAX_INT + 1:
                                    self.cursor.execute(
                                        "INSERT INTO MicroBenchmarkEvent(microBenchmarkMeasurement, event, value) VALUES (?, ?, ?)",
                                        (micro_benchmark_measurement_id, csv_header[i], -1))
                                # Outlier - a value we cannot represent and that should never occur
                                elif item > MAX_INT + 1:
                                    self.cursor.execute(
                                        "INSERT INTO MicroBenchmarkEvent(microBenchmarkMeasurement, event, value) VALUES (?, ?, ?)",
                                        (micro_benchmark_measurement_id, csv_header[i], -2))
                                # Outlier - a value that should not possibly occur
                                elif item == 0:
                                    self.cursor.execute(
                                        "INSERT INTO MicroBenchmarkEvent(microBenchmarkMeasurement, event, value) VALUES (?, ?, ?)",
                                        (micro_benchmark_measurement_id, csv_header[i], -3))
                                else:
                                    self.cursor.execute(
                                        "INSERT INTO MicroBenchmarkEvent(microBenchmarkMeasurement, event, value) VALUES (?, ?, ?)",
                                        (micro_benchmark_measurement_id, csv_header[i], item))


    def stack(self):
        stack_path = self.path.joinpath("stack")
        end_pattern = re.compile("=== [0-9\-]{10} [0-9\:]{8} Command ended with exit code 0 ===")

        for file_name in os.listdir(stack_path):
            with iter(open(stack_path.joinpath(file_name))) as input_file:
                if self.was_skipped(input_file):
                    continue

                start_time, stop_time = self.get_times(input_file)
                benchmark_id = self.add_benchmark("stack", file_name, start_time, stop_time)
                self.cursor.execute("INSERT INTO stackBenchmark(benchmark) VALUES (?)", (benchmark_id,))
                stack_benchmark_id = self.cursor.lastrowid

                result = False
                for line in input_file:
                    if line.startswith("output:"):
                        result = True
                        next(input_file)
                    elif re.search(end_pattern, line) is not None:
                        result = False
                    elif result:
                        parts = re.sub("\s+", ",", line).split(",")
                        if parts[2] == "N/A":
                            allocation = -1
                        else:
                            allocation = int(parts[2])
                        self.cursor.execute(
                            "INSERT INTO stackBenchmarkSymbol(stackBenchmark, symbol, size, allocation) VALUES (?, ?, ?, ?)",
                            (stack_benchmark_id, parts[0], int(parts[1]), allocation))



    def heap(self):
        heap_path = self.path.joinpath("heap")
        for file_name in os.listdir(heap_path):
            if file_name.endswith(".gz"):
                continue

            with open(heap_path.joinpath(file_name)) as input_file:
                if self.was_skipped(input_file):
                    continue

                start_time, stop_time = self.get_times(input_file)
                benchmark_id = self.add_benchmark("heap", file_name, start_time, stop_time)
                for line in input_file:
                    if line.startswith("heaptrack output will be written to "):
                        heaptrack_file = re.search("\".+\"", line).group(0).strip("\"").split("/")[-1]

            output_file = NamedTemporaryFile(delete=False).name
            self.create_flamegraph(str(heap_path.joinpath(heaptrack_file)), output_file)
            self.cursor.execute("INSERT INTO heapBenchmark(benchmark) VALUES (?)", (benchmark_id,))
            heap_benchmark_id = self.cursor.lastrowid
            for line in self.parse_flamegraph(output_file):
                self.cursor.execute(
                    "INSERT INTO heapBenchmarkMeasurement(heapBenchmark, trace, peakAllocation) VALUES (?, ?, ?)",
                    (heap_benchmark_id, line[0], int(line[1])))
            os.remove(output_file)


    @staticmethod
    def create_flamegraph(input_file: str, output_file: str):
        subprocess.run([
            "heaptrack",
            "--analyze",
            "--print-temporary",
            "0",
            "--print-peaks",
            "1",
            "--print-allocators",
            "0",
            "--print-leaks",
            "0",
            "--print-flamegraph",
            output_file,
            "--flamegraph-cost-type",
            "peak",
            input_file
        ], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    @staticmethod
    def parse_flamegraph(input_file: str) -> List[Tuple[str, int]]:
        traces: List[Tuple[str, int]] = []
        with open(input_file, "r") as file:
            for line in file.readlines():
                parts = [x.strip() for x in line.strip().split(";")]
                trace = ";".join(parts[0:-1])
                peak_allocation = int(parts[-1])
                traces.append((trace, peak_allocation))

        return traces


def main(path: Path, database_path: Path):
    parse = Parse(path, database_path)
    try:
        parse.initialize()
        parse.tool_versions()
        parse.sequential()
        parse.parallel()
        parse.micro()
        parse.stack()
        parse.heap()
        parse.commit()
        print("Comitted changes")
    except Exception as exception:
        print("Got exception while parsing")
        print(exception)
        print("Rolling back - no changes has been made to the database")
        parse.rollback()
    finally:
        del parse

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("usage: {} <input_directory> <target_database.sqlite>".format(sys.argv[0]))
        exit(1)
    main(Path(sys.argv[1]), Path(sys.argv[2]))
