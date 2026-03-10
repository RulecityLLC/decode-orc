# EFM Decode Performance Testing

Tracking decode performance of `orc-cli` against the standalone `ld-decode-tools`
(`efm-decoder-f2`, `efm-decoder-f1`, `efm-decoder-d24`) baseline.

---

## System

- **Machine:** titan (sdi)
- **OS:** Linux
- **Build type:** Debug (`CMAKE_BUILD_TYPE=Debug`) — confirmed for both orc and the
  standalone tools via `CMakeCache.txt`

---

## Baseline — standalone `ld-decode-tools` (efm-decoder-*)

These timings were recorded prior to the orc integration and are documented in full
in `docs/efm-golden-sample-baseline.md`.  Each stage is a separate process communicating
via intermediate files on disk.

### Audio decode — `roger_rabbit`

| Stage | Tool | Internal processing time | `real` wall time |
|---|---|---|---|
| EFM → F2  | `efm-decoder-f2` | 14,184 ms (14.18 s) | 0m29.478s |
| F2 → D24  | `efm-decoder-f1` | 32,512 ms (32.51 s) | 0m41.445s |
| D24 → WAV | `efm-decoder-d24` | 3,839 ms (3.84 s)  | 0m15.780s |
| **Total** | | **50,535 ms (50.5 s)** | **~1m26.7s** |

Stage breakdown for EFM → F2:
- Channel to F3: 12,190 ms
- F3 to F2 section: 1,947 ms
- F2 correction: 46 ms

Stage breakdown for F2 → D24:
- F2 to F1: 29,448 ms
- F1 to Data24: 3,063 ms

Stage breakdown for D24 → WAV:
- Data24 to Audio: 3,127 ms
- Audio correction: 711 ms

### Data decode — `DS2_comS1`

| Stage | Tool | Internal processing time | `real` wall time |
|---|---|---|---|
| EFM → F2  | `efm-decoder-f2` | 19,301 ms (19.30 s) | 0m37.797s |
| F2 → D24  | `efm-decoder-f1` | 40,140 ms (40.14 s) | 0m51.215s |
| D24 → BIN | `efm-decoder-d24` | 15,359 ms (15.36 s) | 0m15.955s |
| **Total** | | **74,800 ms (74.8 s)** | **~1m44.97s** |

Stage breakdown for EFM → F2:
- Channel to F3: 15,096 ms
- F3 to F2 section: 2,415 ms
- F2 correction: 1,789 ms

Stage breakdown for F2 → D24:
- F2 to F1: 36,415 ms
- F1 to Data24: 3,724 ms

Stage breakdown for D24 → BIN:
- Data24 to Raw Sector: 14,466 ms
- Raw Sector to Sector: 892 ms

---

## Current — `orc-cli` integrated pipeline

- **Git commit:** `8fad4bf` (tag `v1.0.3-3-g8fad4bf`, branch `issue56-202603`)
- **Commit message:** "Updated all package building to include ezpwd library references
  required for EFM decoder"
- **Command template:**
  ```bash
  time ./build/bin/orc-cli test-projects/<project>.orcprj --process --log-level warn
  ```

Note: orc-cli runs the entire pipeline in-process (no intermediate files written between
stages), so disk I/O between stages is eliminated versus the baseline.

### Audio decode — `roger_rabbit`

| | `real` | `user` | `sys` |
|---|---|---|---|
| orc-cli | **7m2.271s (422.3s)** | 6m59.511s | 0m1.110s |
| Baseline (sum of real) | ~1m26.7s (86.7s) | — | — |
| **Slowdown (wall time)** | **~4.9×** | | |

### Data decode — `DS2_comS1`

| | `real` | `user` | `sys` |
|---|---|---|---|
| orc-cli | **8m20.223s (500.2s)** | 8m16.320s | 0m1.513s |
| Baseline (sum of real) | ~1m44.97s (105.0s) | — | — |
| **Slowdown (wall time)** | **~4.8×** | | |

---

## Observations so far

1. **Both binaries are Debug builds.** The standalone tools and orc-cli were both built
   with `CMAKE_BUILD_TYPE=Debug`, so the build type difference does not explain the
   slowdown.

2. **orc-cli is ~4.8–4.9× slower** in wall-clock time across both test cases despite
   eliminating inter-stage disk I/O.

3. **`user` time closely tracks `real` time** in the orc-cli runs (single-threaded),
   confirming this is CPU-bound work, not I/O-bound.

4. **The baseline ran as 3 separate processes** with intermediate F2 and D24 files
   written to disk between stages. The orc-cli pipeline eliminates that inter-stage I/O,
   which means the actual algorithmic slowdown is even larger than the wall-time ratio
   suggests (the baseline wall times include disk write/read overhead that orc-cli avoids).

5. **Root cause not yet identified.** Likely candidates:
   - Overhead in the orc DAG pipeline architecture (e.g. per-frame/per-section
     Qt signal/slot dispatch, or extra copying through the node graph)
   - The EFM sink receiving data in small chunks via the pipeline rather than
     streaming directly from the file, causing algorithmic inefficiency
   - Logging or statistics bookkeeping at each pipeline step
   - Memory allocation patterns different from the standalone tools

---

## Next steps

- Profile `orc-cli` vs `efm-decoder-*` to identify where the CPU time is spent
- Compare how the EFM sink receives and processes data vs the standalone tool's
  direct file-streaming approach
- Check whether per-frame dispatch through the orc node graph adds significant
  overhead per EFM section processed
