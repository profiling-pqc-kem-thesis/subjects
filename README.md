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
