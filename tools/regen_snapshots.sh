#!/usr/bin/env bash
# Regenerate all in-game screenshot assets used by README and docs.
#
# Run after any UI/i18n/sprite change to keep README screenshots in sync.
# Requires DISPLAY=:1 (or any X11 display); CI can SKIP this — it's
# documentation tooling, not part of the build.

set -uo pipefail
# NOTE: not using `set -e` because snapshot tools sometimes segfault on
# SDL teardown after successfully writing the PNG. The file is correct;
# only the exit code is bad. Skipping `-e` lets the script continue.

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="${ROOT}/build"
OUT="${ROOT}/docs"
SDIR="${BUILD}/src"

if [[ -z "${DISPLAY:-}" ]]; then
    echo "ERROR: DISPLAY not set — need X11 to run snapshot tools." >&2
    exit 1
fi

if [[ ! -x "${SDIR}/snapshot_combat" ]]; then
    echo "Building snapshot tools..."
    cmake --build "${BUILD}" --target snapshot_combat snapshot_help \
                                   snapshot_party snapshot_intro 2>&1 | tail -3
fi

echo "=== generating party / town shots ==="
"${SDIR}/snapshot_party" "${OUT}/tour_01_party.png"

echo "=== generating help overlay shots (4 scenes) ==="
"${SDIR}/snapshot_help"  "${OUT}/tour_02_help_maze.png"  3  # Maze
"${SDIR}/snapshot_help"  "${OUT}/tour_03_help_combat.png" 4  # Combat
"${SDIR}/snapshot_help"  "${OUT}/tour_04_help_castle.png" 2  # Castle

echo "=== generating intro slides (5 pages) ==="
for i in 0 1 2 3 4; do
    "${SDIR}/snapshot_intro" "${OUT}/tour_05_intro_${i}.png" $i
done

echo "=== generating combat shots (3 monsters) ==="
"${SDIR}/snapshot_combat" "${OUT}/tour_06_combat_werdna.png"  25
"${SDIR}/snapshot_combat" "${OUT}/tour_07_combat_vampire.png" 20
"${SDIR}/snapshot_combat" "${OUT}/tour_08_combat_wisp.png"    29

echo ""
echo "Done. Generated $(ls "${OUT}"/tour_*.png 2>/dev/null | wc -l) tour shots."
