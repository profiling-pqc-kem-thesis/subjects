<p align="center">
  <strong><a href="#quickstart">Quick Start</a> | <a href="#documentation">Documentation</a> </strong>
</p>

# subjects
## Instrumented implementations of modern-day KEXs and future KEMs

## Prerequisites

First install your operating system:

* Ubuntu 20.04 on x86
* RHEL 8.3 on Z and POWER

Then ensure you have the following packages installed from your preferred package manager or from source.

* `make`
* `gcc`
* `clang`
* `go`
* `sqlite3`
* `nm`
* `objdump`
* `bash`
* `cmake`
* `boost` (`libboost-all-dev`, `boost-devel`) 1.41.0 or higher
* `libunwind` (`libunwind-dev`)
* `libdwarf` (`libdwarf-dev`)

On s390x build `libunwind` from source:

```sh
git clone https://github.com/libunwind/libunwind
cd libunwind
./autogen.sh
./configure
make
make install
```

Then install heaptrack from source:

```sh
git clone "https://github.com/KDE/heaptrack.git"
cd heaptrack
git checkout v1.2.0
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
sudo make install
```

Then install perforator from source:

```sh
git clone "https://github.com/profiling-pqc-kem-thesis/perforator.git"
cd perforator
go mod edit -replace="github.com/zyedidia/perforator=./"
make
sudo ln -s "/path/to/perforator/perforator" "/usr/local/bin/perforator"
```

## Using

_Note: the recommended workflow is documented in the section "Benchmarking" below._

The environment name is used to name the output, which will be placed in `./data/benchmarks/$environment_name`. The number of seconds to run each parallel benchmark is controlled by the parallel duration. The number of iterations to benchmark for in the sequential benchmark is dictated by sequential iterations.

The script is run in seven steps:

1. Assert pre-conditions
2. Build
3. Collect environmental information
4. Run the heap analysis
5. Run the micro benchmarks
6. Run the sequential tests
7. Run the parallel tests.
8. Run static stack benchmarks

These steps can be controlled using `SKIP_STEPS` like so:

```sh
SKIP_STEPS=1,2,7 ./scripts/benchmark-full.sh | tee benchmark.txt
```

Furthermore, one may control what algorithms are run like so:

```
SKIP_MCELIECE=yes SKIP_NTRU=yes SKIP_DH=yes SKIP_ECDH=yes ./scripts/benchmark-full.sh | tee benchmark.txt
```

Lastly, the number of sequential iterations, parallel iterations, timeout in seconds and sleep between tests are configurable via `SEQUENTIAL_ITERATIONS`, `PARALLEL_ITERATIONS`, `TIMEOUT` and `SLEEP_BETWEEN_TESTS` respectively. The defaults are:

* `TIMEOUT` - 3600 (1h)
* `SEQUENTIAL_ITERATIONS` - 1000
* `PARALLEL_ITERATIONS` - 100000
* `SLEEP_BETWEEN_TESTS` - 1
* `HEAPTRACK_SEQUENTIAL_ITERATIONS` - 10

The output is stored in `./data/benchmarks/$environment_name` and largely looks as follows:

```
data
└── benchmarks
    └── workstation
        ├── build.txt
        ├── duration.txt
        ├── end.txt
        ├── environment.sqlite
        ├── heap
        ├── micro
        ├── parallel
        ├── sequential
        └── start.txt
```

_NOTE: when analysing the results from perforate, know that it will include the results from the `get_global_state`, which includes an extra keypair generation and an encrypt for the KEMs._

## Benchmarking

TODO: Add sudo instructions / paranoia instructions

Starting the benchmark.

While you can run the benchmark script with `sudo` to get the correct monitoring permissions, it's recommended to instead change the `kernel.perf_event_paranoid` level:

```sh
# Add the following row to the file:
# kernel.perf_event_paranoid = -1
sudo vi /etc/sysctl.conf
# Restart
sudo shutdown -r now
```

```sh
# Clone this repository
git clone https://github.com/profiling-pqc-kem-thesis/subjects
cd subjects

# Open a persistent session (you may need to press enter once)
screen

# Clean the entire build
make clean

# Start the benchmark
./scripts/benchmark-full.sh "my-environment-name" | tee benchmark.txt

# Detach from the screen using CTRL+a, d
# that way you may close an SSH connection to a remote machine, etc.
```

Checking the status of the benchmark.

```sh
# List active sessions
screen -ls

# Attach to an active session
screen -r <name / id from screen -ls>
```

Compiling the results.

```sh
tar -xzf archive.tgz ./data/benchmarks
```
