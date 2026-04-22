# NonIntegerGames

## Based on

This repository is a fork of [suguman/NonIntegerGames](https://github.com/suguman/NonIntegerGames),
the original implementation by Bansal, Kavraki, Vardi, and Wells accompanying:

> Suguman Bansal, Lydia E. Kavraki, Moshe Y. Vardi, Andrew Wells.
> *Synthesis from Satisficing and Temporal Goals.*
> AAAI 2022.

## This fork

**Authors:** Raj Chandak and Priyanka Golia, IIT Delhi.

This fork extends the baseline DSLow synthesis solver with three optimizations:

- **B1 — Asymmetric comparator bounds:** replace symmetric max-weight bounds with
  weight-specific bounds `[L, U]`, reducing the theoretical state space by 40–50%
  for asymmetric reward games at no algorithmic cost.

- **B2 — Lazy on-the-fly construction:** materialize only reachable product states
  (ghost states are never allocated), eliminating 74–99% of states the baseline constructs.

- **B3 — Priority-guided exploration with interleaved backward propagation:** replace
  the BFS queue with a min-heap keyed on `(U − comparator)`, expanding states closest
  to winning first. Backward propagation runs incrementally as winning states are found,
  allowing early termination before the full product is built. Discovers the first winning
  state up to 21.7× sooner than BFS.

All new code is in `src/Game.cpp` and `src/Game.h`. The original solver, benchmarks,
and game format are unchanged.

---

## Dependencies

### C++ solver

- C++11 compiler (g++ or clang++)
- [Boost](https://www.boost.org/) library

**Linux:**
```bash
sudo apt-get install libboost-dev
```

**macOS (Homebrew):**
```bash
brew install boost
```

### Python benchmarks

Python 3.6+, no additional packages required.

---

## Build

```bash
cd src/
```

**Linux:**
```bash
make
```

**macOS:**
```bash
make BOOST_INC=$(brew --prefix boost)/include
```

---

## Usage

```
./game -f <game_file> [options]

Required:
  -f <file>     Input game graph file

Discount / precision:
  -df <k>       Discount factor exponent: d = 1 + 2^(-k)   [default: 1]
  -p  <p>       Precision exponent: error bound = 2^(-(p+k)) [default: 1]

Game options:
  -th <n>       Threshold value                             [default: 0]
  -rel <r>      Relation: "geq" or "leq"                   [default: geq]
  -id <n>       Player to check for winning (0 or 1)        [default: 0]

Construction mode (choose one):
  -et           Baseline BFS with early termination
  -hq           Priority-guided heuristic with interleaved propagation

Output:
  -syn          Extract and print winning strategy
  -ws           Print all winning states
  -h            Show help
```

---

## Game file format

```
# Lines starting with # are comments

# State declarations:
<state_id> <player>        # player: 0 (system) or 1 (environment)
<state_id> <player> R      # R marks this state as a reachability goal

# Initial state (single integer):
<state_id>

# Transitions:
<src_state> <dst_state> <weight>
```

See `benchmarks/eg1.txt` for a minimal example.

---

## Example runs

```bash
# Minimal example
./game -f ../benchmarks/eg1.txt -df 1 -p 1 -id 0

# Social distancing 4x4, k=2 — baseline BFS
./game -f ../benchmarks/social_dist/auto_4_4_10_2.txt -df 2 -p 1 -id 1 -et

# Social distancing 4x4, k=2 — priority-guided heuristic
./game -f ../benchmarks/social_dist/auto_4_4_10_2.txt -df 2 -p 1 -id 1 -hq

# Grid World, k=1
./game -f ../benchmarks/grid_world/game.txt -df 1 -p 1 -id 0 -hq

# Conveyor Belt, k=1
./game -f ../benchmarks/conveyor/game.txt -df 1 -p 1 -id 0 -hq
```

The binary prints:
- `Input quant. game: N states` — arena size
- `Reachability game: M states` — product states constructed
- `States@first win: K / M` — states built before first winning state found
- `Game creation / Game play` — wall-clock time

---

## Benchmark domains

### Social Distancing (`benchmarks/social_dist/`)

Two-agent avoidance game on an N×N grid. Asymmetric weights (+10/−2) make this the
primary domain for evaluating the priority-guided heuristic.

Regenerate:
```bash
cd benchmarks/social_dist/
python3 create_game.py <rows> <cols> <pos_reward> <neg_reward>
```

### Grid World (`benchmarks/grid_world/`)

Single-agent collection task on a 10×10 grid with obstacles and temporal subgoals.

### Conveyor Belt (`benchmarks/conveyor/`)

Block-sorting task with near-symmetric weights {−2,−1,0,+1,+2}. Used as a control:
symmetric weights provide little directional signal for the heuristic.

---

## Experimental Results

### Priority-guided first-win improvement

| Benchmark | Reachable states | BFS first-win | HQ first-win | Speedup |
|---|---|---|---|---|
| sd\_4\_4\_10\_2   | 14,568  | 976   | 45  | **21.7×** |
| sd\_6\_6\_10\_2   | 34,574  | 2,730 | 205 | **13.3×** |
| sd\_8\_8\_10\_2   | 51,056  | 5,377 | 459 | **11.7×** |
| sd\_10\_10\_10\_2 | 77,576  | 9,332 | 546 | **17.1×** |
| Grid World        | 20,003  | 4,136 | 505 | **8.2×**  |
| Conveyor Belt     | 130,713 | 88    | 52  | **1.7×**  |

### Lazy construction fill rates

| Benchmark | Arena \|V\| | Comparator range | Theoretical (B1) | Reachable (B2) | Fill |
|---|---|---|---|---|---|
| sd\_4\_4\_10\_2   | 397    | 357 | 141,729   | 14,568  | 10.3% |
| sd\_6\_6\_10\_2   | 2,407  | 357 | 859,299   | 34,574  |  4.0% |
| sd\_8\_8\_10\_2   | 8,093  | 357 | 2,889,201 | 51,056  |  1.8% |
| sd\_10\_10\_10\_2 | 20,572 | 357 | 7,344,204 | 77,576  |  1.1% |
| Grid World        | 20,572 | 131 | 2,694,932 | 20,003  |  0.7% |
| Conveyor Belt     | 14,133 |  35 | 494,655   | 130,713 | 26.4% |

### Asymmetric bounds (B1)

For rewards +10/−2 with k=2: comparator range shrinks from 645 → 357 values (44.7% reduction).

---

## Code structure

```
src/
  Game.cpp / Game.h      Product construction and reachability solver (this fork)
  Graph.cpp / Graph.h    Input game graph loader
  Transition.cpp / .h    Weighted edge
  main.cpp               CLI entry point
  makefile

benchmarks/
  eg1.txt                Minimal example
  social_dist/           Two-agent avoidance games (Bansal et al. + extended instances)
  grid_world/            Single-agent collection task
  conveyor/              Block-sorting task

scripts/
  run.py / run_sd.py / run_conveyor.py   Batch runners
  make_stats.py                          Parse outputs to LaTeX table
```
