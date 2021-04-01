<p align="center">
  <strong><a href="#quickstart">Quick Start</a> | <a href="#documentation">Documentation</a> </strong>
</p>

# subjects
## Instrumented implementations of modern-day KEXs and future KEMs

<a id="quickstart"></a>
## Quick Start

This project holds several implementations of key exchange algorithms recommended for use today, as well as implementations of key encapsulation mechanisms submitted to NIST's standardization process.

The implementations have seen various forms of treatment in terms of optimization, such as AVX2, replaced and optimized library functions etc.

All implementations have been instrumented for measuring CPU cycles, instructions and more.

To build all samples, simply run the following command.

```sh
make all
```

To build a single set of samples, run a command like the following.

```sh
make ecdh
```

To build tests for a set of samples, run a command like the following.

```sh
make -C ecdh tests
```

To build benchmarks for a set of samples, run a command like the following. Note: building the benchmarks rely on [perf](https://github.com/profiling-pqc-kem-thesis/perf) and in turn Linux. The tests are however inteded to be buildable on non-Linux systems.

```sh
make -C ecdh benchmarks
```

Each sample is then available in the corresponding subject's directory. ECDH(E) samples, for example, is available under `ecdh/build`.

Each binary for a benchmark is named according to the following format: `algorithm_parameter-set_implementations_compiler-flags`.

* `algorithm`: the algorithm, such as ecdh, dh, ntru or mceliece
* `parameter-set`: the parameter set as applicable for the algorithm, such as p256, hrss701
* `implementations`: the various implementations used, such as avx2 for an AVX2 optimized implementation, aes for an OpenSSL AES implementation and so on
* `compiler-flags`: a label for the set of compiler flags used, such as `minimal` or `optimized`

Each binary for a test is named according to the following format: `algorithm_parameter-set`.

## Table of contents

[Quickstart](#quickstart)<br/>
[Documention](#documentation)<br />

<a id="documentation"></a>
## Documentation

### Building

To build, the following prerequisites are required:

* `make`
* `gcc` 8 or newer

The various implementations may have further requirements. If so, these are documented in the corresponding section below, under "Available Implementations".

To build all samples and run all tests, simply run the following command.

```sh
make
```

To build and test a specific group of samples, such as ECDH(E), run the following command.

```sh
make ecdh
```

To only run the tests or build a specific version of the algorithm, run one of the following commands.

```sh
make -C ecdh test
make -C ecdh build/ntru_hrss701_ref_test
```

To format all the code, run the following command. The command requires `clang-format` which is easily installed via `brew install clang-format` or `apt install clang-format`.

```sh
make format
```

To use a language server such as `clangd`, run the following command. The command requires `compiledb` which is installed via `pip install compiledb`.

```sh
make compile_commands.json
```

To build the solution in debug mode (using clang, address sanitizer etc.), first execute the following command to setup your environment.

```sh
eval $(make debug-flags)
```

You may now run any build like usual, such as `make test`.

### Identifying hot paths

To identify hot paths, first make sure you've installed Valgrind and KCachegrind:

* `apt install valgrind kcachegrind`
* `brew install --HEAD LouisBrunner/valgrind/valgrind`, `brew install qcachegrind`

Then, build and run the test files (reference implementation) using Valgrind (Callgrind).

```sh
make hotpaths
```

The profiles will be created under the respective implementation's build directory like so:

```
.
└── ntru
    └── build
        ├── ntru_hps4096821_ref_test.profile
        └── ntru_hrss701_ref_test.profile
```

These profiles can be loaded using the following command:

```sh
# Linux
kcachegrind ntru/build/ntru_hps4096821_ref_test.profile

# macOS
qcachegrind ntru/build/ntru_hps4096821_ref_test.profile
```

### Analyzing memory usage

#### Heap usage

Use `heaptrack` (`apt install heaptrack heaptrack-gui`) and run the benchmarks like so:

```
heaptrack ./ecdh/build/ecdh_25519_plain_optimized --sequential --iterations 100000
```

Open the output file with `heaptrack_gui` like so:

```
heaptrack_gui heaptrack.ecdh_25519_plain_optimized.56272.gz
```

Go to the Caller / Callee tab and search for the `crypto_` functions. The peak allocation is the maximum number of bytes the function allocated during a single iteration. The total amount of bytes allocated is dependant on how many times the function is invoked.

### Analyzing processor usage

#### Micro benchmarks

To analyze the performance of the implementations in terms of micro benchmarks, use the [perforator](https://github.com/zyedidia/perforator) tool to leverage the perf API on select methods.

```sh
sudo perforator --summary --csv --group cpu-cycles,instructions -r crypto_kem_keypair -r crypto_kem_enc -r crypto_kem_dec -- ./ntru/build/ntru_hrss701_avx2-optimized --sequential --encrypt --iterations 1000
```

```
fething global state for benchmark 'encrypt', 'build/ntru_hrss701_avx2-optimized'
running benchmark 'encrypt' for 'build/ntru_hrss701_avx2-optimized'
progress:  100%
encrypt build/ntru_hrss701_avx2-optimized average (of 1000 iterations): 0.058465ms
region,cpu-cycles,instructions,time-elapsed
crypto_kem_keypair,1225075,2949677,477.354µs
crypto_kem_enc,215541,314572,49.719µs
...
```

### Running the automated benchmarks

The script `./scripts/benchmark-full.sh` automates the entire process of running the script. It has the following prerequisites:

* `make`
* `gcc`
* `clang`
* `go`
* `sqlite3`
* [perforator](https://github.com/zyedidia/perforator)
* `heaptrack`

It is run like so:

```sh
./scripts/benchmark-full.sh <environment name>
```

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
SKIP_STEPS=1,2,7 ./scripts/benchmark-full.sh
```

Furthermore, one may control what algorithms are run like so:

```
SKIP_MCELIECE=yes SKIP_NTRU=yes SKIP_DH=yes SKIP_ECDH=yes ./scripts/benchmark-full.sh
```

Lastly, the number of sequential iterations, parallel iterations and timeout in seconds is configurable via `SEQUENTIAL_ITERATIONS`, `PARALLEL_ITERATIONS` and `TIMEOUT`, respectively.

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

### Available Implementations

#### ECDH(E)

Ephemeral Elliptic-Curve Diffie-Hellman is implemented as a modern-day algorithm as recommended by NIST, MDN, IETF and the EU-funded PQCRYPTO. The implementation builds exclusively on the OpenSSL library.

The curves implemented are `P-256` and `Curve25519` (in some contexts known as `x25519`).

The implementations are assumed to be optimized for the underlying platform via OpenSSL and the compiler.

Refer to [the report](https://github.com/profiling-pqc-kem-thesis/report) for further details.

The resulting binaries for benchmarks are documented in the table below.

| Name | Description |
| :--: | ----------- |
| `ecdh_p256_plain_optimized` | ECDH on P-256 implemented using OpenSSL and compiled with `-O3`, `-march=native` |
| `ecdh_25519_plain_optimized` | ECDH (x25519) on Curve25519 implemented using OpenSSL and compiled with `-O3`, `-march=native` |

To build the ECDH implementations, OpenSSL is required. It is also required that the include headers and libraries are in the correct location, or that their paths are specified in the `CPPFLAGS` and `LDFLAGS` environment variables.

#### DH(E)

Ephemeral Diffie-Hellman is implemented as a modern-day algorithm as recommended by NIST, MDN, IETF and the EU-funded PQCRYPTO. The implementation builds exclusively on the OpenSSL library.

The implementation is assumed to be optimized for the underlying platform via OpenSSL and the compiler.

Refer to [the report](https://github.com/profiling-pqc-kem-thesis/report) for further details.

The resulting binaries for benchmarks are documented in the table below.

| Name | Description |
| :--: | ----------- |
| `dh` | DH implemented using OpenSSL and compiled with `-O3`, `-march=native` |

To build the DH implementations, OpenSSL is required. It is also required that the include headers and libraries are in the correct location, or that their paths are specified in the `CPPFLAGS` and `LDFLAGS` environment variables.

#### NTRU

NTRU is implemented as a post-quantum algorithm as submitted to NIST's standardization process. The implementations are based solely on the [provided reference and optimized implementation](https://github.com/profiling-pqc-kem-thesis/ntru).

The implemented parameter sets are `HRSS701`, `HPS2048509`, `HPS2048677` and `HPS4096821`, although only `HRSS701` and `HPS4096821` is exposed via the Makefile.

To build the NTRU implementations, OpenSSL is required. It is also required that the include headers and libraries are in the correct location, or that their paths are specified in the `CPPFLAGS` and `LDFLAGS` environment variables.

## Disclaimer

The implementations are not tested nor validated in terms of security. It is possible that security features of the implementations have been limited after changes made by us. The focus in this context is on the performance behavior of the algorithms, not the security. The implemented tests only test the general function of the algorithms in this context and are not extensive.

## License

For details on the NTRU implementation, please refer to this repository: <https://github.com/profiling-pqc-kem-thesis/ntru>.

For details on the Classic McEliece implementation, please refer to this repository: <https://github.com/profiling-pqc-kem-thesis/classic-mceliece>.

For details on the XKCP library, please refer to this repository: <https://github.com/profiling-pqc-kem-thesis/XKCP>.

For details on the perf library, please refer to this repository: <https://github.com/profiling-pqc-kem-thesis/perf>.
