# MyShield — Geant4 cosmic-ray shielding (GCR / SEP)

## Build (Windows, CMake)

From `Lera`:

```powershell
cmake -S . -B build
cmake --build build --config Release
```

## Run modes

Interactive (OpenGL + UI):

```powershell
.\build\Release\MyShield.exe
```

Batch (execute a macro and exit; no session UI):

```powershell
.\build\Release\MyShield.exe macros\run_stats.mac
```

Single-config generators (e.g. Ninja): `.\build\MyShield.exe`.

## Cosmic-ray source (macros)

- `/cosmic/isotropic false` — hemisphere toward **−Z** (default in `run_*.mac`).
- `/cosmic/isotropic true` — isotropic **4π** directions.
- **`/cosmic/useSkyDisk true`** (default) — vertex on a **disk at z = +11 m**, **r ≤ 2 m** (uniform in area), so most primaries **geometrically intersect** the habitat cylinder and the **“aimed at habitat”** counters are meaningful.
- **`/cosmic/useSkyDisk false`** — vertex on a **sphere r = 10.5 m**; with random direction the solid angle on the habitat is tiny, so **“Primary aimed at habitat”** is usually **0** even when secondaries still hit the habitat.

Mixture: **70% GCR** (power law E⁻²·⁷, p / α / ⁵⁶Fe per spec) and **30% SEP** (E⁻²·⁰; SEP protons include **0.2%** photon share subtracted from proton weight).

## Physics list

The app tries reference lists in order: **FTFP_BERT_HP**, **Shielding**, **QGSP_BIC_HP**. If none is registered, it falls back to **FTFP_BERT** (no HP). For neutron capture on **B-10**, use an install built with HP data and prefer `_HP` or **Shielding**.

## Geometry (Z = ship axis)

Habitat: air cylinder **R = 2 m**, half-length **5 m**. Passive shell: Al **2.0–2.003 m**, PE **2.003–2.006 m**, same half-length. Solenoid MgB₂ **2.5–2.55 m**, half-length **6 m**. Four MgB₂ gradient rings (torus R = 6.5 m, tube 0.1 m) at Z = ±4.5, ±1.5 m. Boron in MgB₂ is **20% B-10 / 80% B-11** by isotope abundance.

## Magnetic field

**r ≤ 2.5 m:** Bz = **2 T** (uniform). **2.5 m < r < 6.5 m:** Bz = **2 − (r − 2.5)** T (with **r** in metres). **r ≥ 6.5 m** or **Bz < 0:** Bz = 0; Bx = By = 0.

## Metrics (end of run)

- **Shielding efficiency (aimed primary, %):** among primaries whose ray from the vertex along **p̂** intersects the **finite** habitat cylinder (including end caps), fraction that **never** enter the habitat volume, times 100.
- **Shielding efficiency (charged, %):** same, restricted to **|Q| > 0** primaries (excludes gammas).
- **Events with n/γ (track ≠ 1) entering habitat:** percentage of events where a **secondary** neutron or gamma enters the habitat (passive / neutral channel indicator).
- **Absorbed dose in habitat:** sum of `edep` in the habitat logical volume divided by **air** mass in that cylinder (**Gy** to air, not tissue). **Equivalent dose estimate:** Gy × **Q** (default **Q = 2.5**, adjustable in code via `RunAction::SetQualityFactor`).
- **Neutron captures in MgB₂:** neutrons stopped with a **Capture** process in a volume whose material is **MgB2**.
- **`spectrum_passed_habitat.csv`:** histogram of **kinetic energy at first boundary crossing into habitat** (any particle), log bins from **0.1 MeV** to **10⁷ MeV**.

## Outputs in `simulations/`

After each completed run, the app writes **`run_summary.txt`** and **`spectrum_passed_habitat.csv`** either directly under **`simulations/`** or under **`simulations/<tag>/`** if you set **`/sim/outputTag <tag>`** in your macro (before `beamOn`). See **`simulations/README.md`** (Russian) for a full explanation of each line.

## Macros

| Macro | Purpose |
|--------|--------|
| `macros/vis.mac` | Interactive vis + trajectory style |
| `macros/geometry_vis.mac` | Geometry only |
| `macros/run_1.mac` / `run_10.mac` / `run_100.mac` | Short runs |
| `macros/run_stats.mac` | 1000 events, quiet |
| `macros/run_10k.mac` / `run_50k.mac` | 10k / 50k events |

After opening the UI, you can still run `/control/execute macros/run_10.mac`.

## Serial run manager

The executable uses **serial** `G4RunManager` for stable interactive visualization on Windows.
