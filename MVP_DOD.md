# MVP Definition of Done

Cross-reference of GDD DoD requirements to implementation locations.
Manual smoke-tests are listed at the bottom and must be re-run before
shipping the prototype.

## Checklist

- [x] **1 combat arena** — fixed 1280x720 playfield in
  [`RunScreen.cpp`](src/screens/RunScreen.cpp) (`WindowWidth`,
  `WindowHeight`).
- [x] **Run lasts 3 minutes** — `RunDurationSeconds = 180.0f` and
  `m_runTimeLeft` ticked in `update()`. Stage 11.
- [x] **3 enemy archetypes** — `EnemyArchetype { Melee, Ranged, Caster }`
  in [`Components.h`](src/ecs/Components.h); spawn helpers and per-archetype
  AI in [`RunScreen.cpp`](src/screens/RunScreen.cpp). Stages 5 and 9.
- [x] **6 level-up perks** — definitions in
  [`Perks.cpp`](src/game/Perks.cpp), application in
  `RunScreen::applyPerk`. Stage 8.
- [x] **4 defeat passives** — definitions in
  [`DefeatPassives.cpp`](src/game/DefeatPassives.cpp), modifiers applied
  on run start in `RunScreen` constructor. Stage 13.
- [x] **Base stub showing live data** —
  [`BaseScreen.cpp`](src/screens/BaseScreen.cpp) renders Stats,
  Resources and Defeat passives blocks from `PersistentState`. Stages 12,
  14, 15.
- [x] **Sprite/texture rendering with placeholder fallback** —
  [`AssetManager.cpp`](src/assets/AssetManager.cpp) generates an in-memory
  fallback image when a file is missing; `RunScreen::renderEntities`
  consumes the sprite pipeline.
- [x] **HP defeat** — `DefeatReason::Hp` set in `RunScreen::update` when
  `currentHp <= 0`. Stage 5 / 10.
- [x] **Arousal defeat** — `DefeatReason::Arousal` set when
  `arousal >= max`; HP wins the tie. Stage 10.
- [x] **Stop screen** — `m_isStopped` overlay with `1) Continue (stub)`
  and `2) Exit to Base`. Stage 11.
- [x] **100% resources on stop exit** — `applyRunResult(raw, Stopped)`
  in [`PersistentState.cpp`](src/game/PersistentState.cpp). Stage 12.
- [x] **30% resources on defeat** — `floor(raw * 0.3)` (constant
  `DefeatGrantFraction = 0.3f`). Stage 12.
- [x] **Persistent state with save / load** —
  [`PersistentState.cpp`](src/game/PersistentState.cpp) `saveToFile` /
  `loadFromFile`, called from `RunScreen::commitRunResultIfNeeded` and
  `Application::Application` respectively. Stage 14.
- [x] **MSVC build** — `cmake --preset msvc` +
  `cmake --build --preset msvc-debug --clean-first` succeeds without
  warnings under `/W4 /permissive-`.
- [ ] **MinGW best-effort build** — `cmake --preset mingw` currently
  fails at configure: `find_package(SFML 2.6 ...)` cannot locate the
  MinGW SFML distribution at the path expected by `CMakePresets.json`
  (`../SFML-2.6.2-windows-gcc-13.1.0-mingw-64-bit/lib/cmake/SFML`).
  This is a missing-distribution issue, not a code-portability issue;
  installing the MinGW SFML drop at that path is expected to clear it.

## Manual smoke checklist (run before shipping)

These cannot be exercised from a build script and must be verified by
hand:

- [ ] Delete `save.txt`. Launch. Stats reads `Runs: 0`,
  `Total resource: 0`. After one Stop run, save file appears, base
  reflects new totals.
- [ ] Restart the process. Persisted data still on the base.
- [ ] Take a Defeat (HP) — overlay shows `DefeatHp`, granted ==
  `floor(raw * 0.3)`, defeat passive name appears. `hpDefeatsCount`
  increments on base.
- [ ] Take a Defeat (Arousal) — same behaviour with
  `arousalDefeatsCount`.
- [ ] Trigger a level-up; verify the picked perk affects `Mult(...)`
  on the HUD.
- [ ] Across several runs, observe that all 6 perks can be offered.
- [ ] Across several defeats, accumulate at least one of each of the
  4 passives in the base list.
- [ ] In an active run: F6 spawns a stress wave; F7 toggles the
  system-timing HUD line. Both are no-ops on overlays.
- [ ] Corrupt `save.txt` (drop the `version` line, or append garbage).
  Launch — stderr logs `[save] corrupted, using defaults`, app starts
  with defaults, no crash.
