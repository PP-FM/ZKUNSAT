# ZKUNSAT

Zero-knowledge proof of propositional unsatisfiability (UNSAT) from a
resolution refutation, built on VOLE-based ZK (the CCS 2022 system). The
prover convinces the verifier that a CNF formula is unsatisfiable — by proving
in zero knowledge that it knows a valid resolution refutation — without
revealing the formula or the proof.

This branch targets the **latest emp toolkit** (the post-refactor generation:
detemplated `IOChannel`/`BoolIO`, single-threaded backend, and the native
`ZKRam`/`ZKROM` "two shuffles" memory). The clause store now uses emp-zk's
`ZKROM`, and the clause-polynomial machinery runs on the unified bool backend's
authenticated F(2^128) (`f2k`) wire ops instead of a bespoke `F2kOSTriple`.

## Dependencies

- A C++17 compiler and CMake ≥ 3.21 (Ubuntu 22.04+, recent macOS) and Python 3
- OpenSSL (libcrypto) — pulled in by the emp installer below
- [NTL](https://libntl.org/) and [GMP](https://gmplib.org/) — the prover's
  cleartext `GF2EX` polynomial arithmetic (ZKUNSAT-specific, not pulled in by
  emp):

```bash
# macOS (Homebrew)
brew install ntl gmp

# Ubuntu / Debian
sudo apt-get install -y libntl-dev libgmp-dev
```

## Install the emp toolkit (latest)

ZKUNSAT links three emp libraries. Use the canonical emp installer
([emp-readme](https://github.com/emp-toolkit/emp-readme#installation)), which
installs the dependencies and tracks the **`main`** (latest) line of each repo:

```bash
# from the directory where you want the emp repos checked out
curl https://raw.githubusercontent.com/emp-toolkit/emp-readme/main/scripts/install.py -O
python3 install.py --deps --tool --ot --zk
```

This installs system-wide (CMake package configs under the default prefix), so
ZKUNSAT finds the libraries with no extra flags.

> **emp-zk must be the new generation.** ZKUNSAT requires the refactored emp-zk
> that provides the native `ZKRam`/`ZKROM` ("two shuffles") memory and the
> `emp::ZKBoolBase::f2k_input(block)` primitive (it commits clause-polynomial
> coefficients as authenticated field elements). If `install.py --zk` resolves
> to an older `emp-zk`, install the current `main` instead
> (`python3 install.py --zk=main`, or build that branch from source).

If you prefer **not** to install system-wide, point ZKUNSAT at the sibling
build trees instead (see below).

## Build ZKUNSAT

```bash
git clone https://github.com/PP-FM/ZKUNSAT.git
cd ZKUNSAT
cmake -S . -B build
cmake --build build -j
```

If the emp libraries are not installed system-wide, pass their build trees at
configure time:

```bash
cmake -S . -B build \
  -Demp-tool_DIR=/path/to/emp-tool/build \
  -Demp-ot_DIR=/path/to/emp-ot/build \
  -Demp-zk_DIR=/path/to/emp-zk/build
```

The build produces a single executable, `build/test`.

## Run

ZKUNSAT is a two-party protocol. **Party 1 is the prover (ALICE)** and holds
the formula and its refutation; **party 2 is the verifier (BOB)**.

```
./build/test <party> <port> <prover_ip> <proof_file>
```

- `<party>` — `1` for the prover, `2` for the verifier
- `<port>` — TCP port to use
- `<prover_ip>` — the prover's IP (used by the verifier to connect; ignored by
  the prover, which listens). Use `127.0.0.1` for a local run.
- `<proof_file>` — path to the unfolded refutation (`*.prf.unfold`). The prover
  reads it; the verifier learns only its size over the network, but still needs
  the argument present.

Local example (two terminals, or background the prover):

```bash
# terminal 1 — prover
./build/test 1 12345 127.0.0.1 prover_backend/example_small.prf.unfold

# terminal 2 — verifier
./build/test 2 12345 127.0.0.1 prover_backend/example_small.prf.unfold
```

On success the verifier prints `check_zero succeed!!!` and both sides print the
total communication and `----end----`. A tampered or invalid refutation makes
the verifier abort with `check_zero failed!`.

## Proof files

`*.prf.unfold` files are line-oriented records of the resolution chain
(`index: … clause: … support: … pivot: … end: …`, with an optional `DEGREE:`
line). The scripts under [`prover_backend/`](prover_backend) turn a CNF plus a
DRAT proof into this format (`preprocess.sh`, `unfold_proof.py`,
`ExtendProof.py`, `Sort.py`); `prover_backend/example_small.prf.unfold` is a
ready-to-run sample.
