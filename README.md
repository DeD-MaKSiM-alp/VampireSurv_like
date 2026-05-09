# VampireSurv-like prototype

A small `Vampire Survivors`-style MVP prototype written in C++17 / SFML 2.6.
The full GDD is in `GDD_for_MVP.md`; the build was driven by the staged plan
in `prototype_development_plan.md` (stages 0 through 17).

The loop is `Base -> Run (3 minutes) -> Result -> Base`. Each run drops
resource pickups and XP; defeats hand out persistent passives that scale
incoming damage and player mobility. Progress is saved to `save.txt` next
to the executable.

## Requirements

- Windows 10/11
- Visual Studio 2022 (MSVC 17, x64) — primary build
- CMake 3.21+
- SFML 2.6.2 binaries laid out alongside the repository:
  - `../SFML-2.6.2-windows-vc17-64-bit/` for MSVC
  - `../SFML-2.6.2-windows-gcc-13.1.0-mingw-64-bit/` for MinGW (best-effort)

The two presets in `CMakePresets.json` point at those directories.

## Build (MSVC)

```
cmake --preset msvc
cmake --build --preset msvc-debug
```

The post-build step copies the SFML DLLs next to the executable.

## Build (MinGW, best-effort)

```
cmake --preset mingw
cmake --build --preset mingw-debug
```

This path is best-effort and depends on a matching SFML-MinGW
distribution being present at the expected path. MSVC remains the
supported target.

## Run

Executable lives at `build/msvc/Debug/VampireSurvLike.exe`. Launch it from
the repository root so that `assets/` and `save.txt` resolve relative to
the working directory.

On first launch `save.txt` does not exist — the game logs
`[save] no file, fresh state` to stderr and starts with default state.
A corrupted save logs `[save] corrupted, using defaults` and falls back
gracefully.

## Controls

### Base
- `Enter` — start a run.

### Run (active simulation)
- `WASD` / arrow keys — move.
- `Esc` — leave the run early. Counts as `Stopped` (full resource grant).
- `F6` — debug: spawn a stress wave (30 melee + 15 ranged + 10 caster) at
  random positions. Cumulative.
- `F7` — debug: toggle per-system timing line in the HUD.

### Level-up overlay
- `1` / `2` / `3` — pick one of three offered perks.

### Stop overlay (timer hit zero or all waves cleared)
- `1` — Continue (MVP stub no-op).
- `2` / `Esc` — exit to the result overlay.

### Defeat overlay
- `Enter` — proceed to the result overlay.

### Result overlay
- `Enter` — return to base. Other keys are ignored.

## What's in MVP

- One combat arena (1280x720), 3-minute run with three scripted waves
  (t=0 / t=60 / t=120).
- Three enemy archetypes with full stats from the GDD: `Melee`
  (HP 40, contact damage 8), `Ranged Archer` (HP 30, projectile damage 7,
  speed 280, cooldown 1.4 s) and `Caster` (HP 50, projectile damage 10
  + 8 arousal damage, speed 230, cooldown 1.8 s).
- Player: HP 100, Arousal 0..100, autoattack 16 dmg / 0.55 s / 260 px
  range, six level-up perks (Power I, Attack Speed I, Vitality I, Calm
  Mind, Momentum, Range I).
- Two independent defeat scales (HP and Arousal), tracked with a
  `DefeatReason` so the result overlay and base UI can label outcomes.
- Resource economy: enemies drop 1 / 2 / 3 resource by archetype. Stop
  exit grants 100% raw, defeat grants `floor(raw * 0.3)`. Single
  `applyRunResult` invocation per run.
- Four defeat passives with multiplicative stacking
  (`pow(base, count)`): Fractured Nerves (+10% incoming HP),
  Sensitive Skin (+15% incoming arousal), Heavy Steps (-8% move speed),
  Shaky Focus (+12% attack interval).
- Persistent state on disk in a versioned line-based `save.txt`. Tracks
  total resource, last-run summary, defeat passives and run / HP-defeat
  / arousal-defeat counters.
- Result overlay between run and base showing outcome / raw / granted
  / passive granted (when applicable).
- Structured base hub stub showing Stats, Resources, last-run result
  and grouped passive list.
- Debug HUD with FPS, entity / enemy / projectile / pickup counts,
  player multipliers, and an optional system-timing line (F7).

## Out of scope

The following GDD items are intentionally NOT in the MVP and remain for
future work:
- Base building / upgrade tree.
- Good / bad endings or narrative branching.
- Meta-progression beyond the in-memory total resource and persistent
  passives.
- Procedurally generated maps.
- Multithreaded ECS / spatial grids (decision documented as a comment
  in `RunScreen.cpp`).
- Final art assets (placeholder / fallback textures only).
- Multiplayer or modding.

## DoD checklist

The full Definition of Done checklist with cross-references to source
locations lives in [`MVP_DOD.md`](MVP_DOD.md).
