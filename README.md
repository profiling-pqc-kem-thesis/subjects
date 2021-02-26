<p align="center">
  <strong><a href="#quickstart">Quick Start</a></strong>
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
make build
```

Each sample is then available in the corresponding subject's directory. ECDH(E) samples, for example, is available under `ecdh/build`.

Each binary is named according to the following format: `algorithm_parameter-set_implementations_compiler-flags`.

* `algorithm`: the algorithm, such as ecdh, dh, ntru or mceliece
* `parameter-set`: the parameter set as applicable for the algorithm, such as p256, hrss701
* `implementations`: the various implementations used, such as avx2 for an AVX2 optimized implementation, aes for an OpenSSL AES implementation and so on
* `compiler-flags`: a label for the set of compiler flags used, such as `minimal` or `optimized`

## Table of contents

[Quickstart](#quickstart)<br/>
[Documention](#documentation)<br />

<a id="documentation"></a>
## Documentation

### Available Implementations

#### ECDH(E)

Elliptic-Curve Diffie-Hellman (Ephemeral) is implemented as a modern-day algorithm as recommended by NIST, IETF and the EU. The implementation builds exclusively on the OpenSSL library.

The curves implemented are `P-256` and `Curve25519` (in some contexts known as `x25519`).

The implementations are assumed to be optimized for the underlying platform via OpenSSL and the compiler.

Refer to [the report](https://github.com/profiling-pqc-kem-thesis/report) for further details.
