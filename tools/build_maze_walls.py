#!/usr/bin/env python3
"""Programmatic wall builder for wiz1_mazes.json.

We describe each floor with high-level primitives:
    - room(x0, y0, x1, y1)      : rectangular room, walled on all sides
    - corridor_h(y, x0, x1)     : horizontal corridor cells (walls above/below)
    - corridor_v(x, y0, y1)     : vertical corridor cells (walls left/right)
    - door(x, y, side)          : punch a door through a wall
    - hdoor(x, y, side)         : hidden door
    - oneway(x, y, side)        : one-way door
    - block(x0, y0, x1, y1)     : solid impassable block (e.g. "Wall" zone shaded grey)
    - dark(x0, y0, x1, y1)      : mark cells as dark zones
    - open_passage(x, y, side)  : ensure a wall between two cells is open

Then a Grid is materialised to per-cell n/e/s/w entries, merged with feature cells
(stairs, elevators, markers, etc.) from the existing JSON, and written back out.

Coordinates: x:0-19 left→right; y:0-19 bottom→top (matches PDF + existing JSON).
"""
from __future__ import annotations

import json
from pathlib import Path
from typing import Iterable

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "assets" / "data" / "wiz1_mazes.json"
SIZE = 20

OPPOSITE = {"n": "s", "s": "n", "e": "w", "w": "e"}
DELTAS = {"n": (0, 1), "s": (0, -1), "e": (1, 0), "w": (-1, 0)}


class Grid:
    def __init__(self) -> None:
        # Each cell stores wall sides. Sides default to "open"; we explicitly
        # mark walls. After build we emit cells only when at least one side is
        # non-open OR a feature is present.
        self.cells: dict[tuple[int, int], dict[str, str]] = {}
        self.dark: set[tuple[int, int]] = set()
        # Blocked cells: solid rock, treat as cell with all 4 walls (impassable).
        self.blocked: set[tuple[int, int]] = set()

    # ---- low-level wall plumbing ---------------------------------------
    def _set_side(self, x: int, y: int, side: str, value: str) -> None:
        if not (0 <= x < SIZE and 0 <= y < SIZE):
            return
        cell = self.cells.setdefault((x, y), {})
        cell[side] = value
        # Mirror to neighbour for consistency.
        dx, dy = DELTAS[side]
        nx, ny = x + dx, y + dy
        if 0 <= nx < SIZE and 0 <= ny < SIZE:
            ncell = self.cells.setdefault((nx, ny), {})
            ncell[OPPOSITE[side]] = value

    def wall(self, x: int, y: int, side: str) -> None:
        self._set_side(x, y, side, "wall")

    def door(self, x: int, y: int, side: str) -> None:
        self._set_side(x, y, side, "door")

    def hdoor(self, x: int, y: int, side: str) -> None:
        self._set_side(x, y, side, "hidden_door")

    def oneway(self, x: int, y: int, side: str) -> None:
        self._set_side(x, y, side, "one_way")

    def open_passage(self, x: int, y: int, side: str) -> None:
        self._set_side(x, y, side, "open")

    # ---- room primitives -----------------------------------------------
    def room(self, x0: int, y0: int, x1: int, y1: int) -> None:
        """Rectangular room, walls on the outer perimeter. Interior is open."""
        if x0 > x1: x0, x1 = x1, x0
        if y0 > y1: y0, y1 = y1, y0
        # South wall: bottom row, side='s'
        for x in range(x0, x1 + 1):
            self.wall(x, y0, "s")
            self.wall(x, y1, "n")
        for y in range(y0, y1 + 1):
            self.wall(x0, y, "w")
            self.wall(x1, y, "e")

    def corridor_h(self, y: int, x0: int, x1: int) -> None:
        """Horizontal 1-cell-tall corridor from x0..x1 inclusive at row y."""
        if x0 > x1: x0, x1 = x1, x0
        for x in range(x0, x1 + 1):
            self.wall(x, y, "n")
            self.wall(x, y, "s")
        self.wall(x0, y, "w")
        self.wall(x1, y, "e")

    def corridor_v(self, x: int, y0: int, y1: int) -> None:
        if y0 > y1: y0, y1 = y1, y0
        for y in range(y0, y1 + 1):
            self.wall(x, y, "w")
            self.wall(x, y, "e")
        self.wall(x, y0, "s")
        self.wall(x, y1, "n")

    def block(self, x0: int, y0: int, x1: int, y1: int) -> None:
        """Solid impassable rock zone. All 4 walls on each cell, marked blocked."""
        if x0 > x1: x0, x1 = x1, x0
        if y0 > y1: y0, y1 = y1, y0
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                self.wall(x, y, "n")
                self.wall(x, y, "s")
                self.wall(x, y, "e")
                self.wall(x, y, "w")
                self.blocked.add((x, y))

    def shade_dark(self, x0: int, y0: int, x1: int, y1: int) -> None:
        if x0 > x1: x0, x1 = x1, x0
        if y0 > y1: y0, y1 = y1, y0
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                self.dark.add((x, y))

    # ---- emit ---------------------------------------------------------
    def to_cells(self) -> list[dict]:
        out: list[dict] = []
        for y in range(SIZE):
            for x in range(SIZE):
                c = self.cells.get((x, y), {})
                if not c and (x, y) not in self.dark and (x, y) not in self.blocked:
                    continue
                row: dict = {"x": x, "y": y}
                # only emit non-open sides for compactness
                for side in ("n", "e", "s", "w"):
                    v = c.get(side)
                    if v and v != "open":
                        row[side] = v
                if (x, y) in self.dark:
                    row["dark"] = True
                if (x, y) in self.blocked:
                    row["blocked"] = True
                out.append(row)
        return out


# ===========================================================================
# Floor descriptions — distilled from Rob Craig PDF + tk421 cross-reference.
# We aim for high structural fidelity on B1F-B4F and structural correctness on
# B5F-B10F. Doors/rooms placed to match visible PDF features.
# ===========================================================================

def build_b1() -> Grid:
    """B1F Proving Grounds Level 1. Two large room clusters separated by a
    central dark/wall column at x≈9-10. The west cluster has the start (0,0)
    and stairs_down (0,9); the east cluster has the K (key) at 14,17."""
    g = Grid()

    # Dark column dividing the level (x=9 is the "Dark" shaded strip on PDF).
    g.shade_dark(9, 0, 9, 19)
    # Inner dark patches (already in JSON for x=13-15,y=13 and x=9,y=4-7).
    g.shade_dark(13, 13, 15, 13)

    # --- West cluster (x=0..8) ---
    # Big perimeter: walk-around corridor at outer rim.
    # Northwest room block 0-8 x 14-19 (top-left dense rooms).
    g.room(0, 18, 0, 19)
    g.room(1, 18, 1, 19)
    g.room(2, 18, 3, 19); g.door(2, 19, "s")
    g.room(4, 18, 5, 19); g.door(4, 19, "s")
    g.room(6, 18, 8, 19); g.door(7, 19, "s")
    # Row y=17 corridor running east-west
    g.corridor_h(17, 0, 8)
    g.door(0, 18, "s"); g.door(1, 18, "s")

    # Mid-west cluster (the cross-room around 3-4, 14-16)
    g.room(2, 14, 4, 16)
    g.door(3, 16, "n"); g.door(2, 15, "w")
    # Side rooms
    g.room(0, 14, 1, 16); g.door(1, 15, "e")
    g.room(5, 15, 7, 16); g.door(5, 15, "w")
    g.room(5, 13, 6, 14); g.door(6, 14, "n")
    # Central small "+"-room at (3-4, 12-13)
    g.room(3, 12, 4, 13); g.door(3, 13, "n"); g.door(4, 12, "s")

    # Big west room block 2-5 x 9-11
    g.room(2, 9, 5, 11)
    g.door(3, 11, "n"); g.door(5, 10, "e")
    g.room(6, 10, 7, 11); g.door(6, 10, "w")
    g.room(0, 10, 1, 12); g.door(1, 12, "n")

    # Long horizontal corridor labelled "1" at y=8
    g.corridor_h(8, 1, 8)
    # Stairs_down room at (0,9)
    g.room(0, 8, 0, 9); g.door(0, 8, "e")

    # Lower-west maze (y=0..7)
    g.room(0, 0, 0, 7); g.door(0, 7, "n")  # start column
    g.room(1, 0, 8, 1)  # bottom band
    g.door(4, 1, "n"); g.door(7, 1, "n")
    g.room(2, 2, 3, 4); g.door(2, 4, "n")
    g.room(5, 2, 7, 4); g.door(5, 4, "n"); g.door(7, 2, "e")
    g.room(2, 5, 4, 7); g.door(4, 7, "e")
    g.room(5, 5, 8, 7); g.door(6, 7, "n"); g.door(8, 6, "e")

    # --- East cluster (x=11..19) ---
    # Top row of mini-rooms
    g.room(11, 18, 11, 19); g.door(11, 18, "s")
    g.room(12, 17, 13, 19); g.door(13, 18, "e"); g.door(12, 17, "s")
    g.room(14, 18, 15, 19); g.door(14, 18, "s")  # K room (14,17)
    g.room(16, 18, 17, 19); g.door(17, 18, "s")
    g.room(18, 18, 19, 19); g.door(19, 18, "s")

    # Vertical corridor at x=11 going down
    g.corridor_v(11, 10, 17)
    # East rooms band y=14..16
    g.room(12, 14, 13, 16); g.door(13, 16, "n"); g.door(13, 14, "s")
    g.room(15, 13, 16, 16); g.door(15, 14, "w"); g.door(16, 16, "n")
    g.room(17, 14, 19, 16); g.door(17, 14, "s"); g.door(19, 15, "e")

    # Middle east band y=10..13
    g.room(12, 10, 13, 13); g.door(12, 13, "n"); g.door(13, 10, "s")
    g.room(14, 11, 16, 12); g.door(14, 12, "w"); g.door(16, 11, "e")
    g.room(17, 10, 19, 13); g.door(17, 13, "n"); g.door(19, 12, "e")

    # Lower east band y=6..9
    g.room(11, 6, 13, 9); g.door(11, 9, "w"); g.door(13, 6, "s")
    g.room(14, 6, 16, 9); g.door(15, 6, "s"); g.door(16, 9, "n")
    g.room(17, 6, 19, 9); g.door(17, 9, "n"); g.door(19, 7, "e")

    # The bottom-east big room with markers 1/1' at (12-16, 0-5)
    g.room(11, 0, 12, 5); g.door(11, 5, "n"); g.door(12, 0, "s")
    g.room(13, 1, 16, 5)  # 1' big room
    g.door(13, 5, "n"); g.door(16, 1, "e")
    g.room(17, 0, 19, 5); g.door(17, 5, "n"); g.door(19, 4, "e")

    return g


def build_b2() -> Grid:
    """B2F. Upper-left has the surrounded K (key) room; centre has E elevator,
    U up-stair, P (Pit at 11,11), D down-stair area at (17,15)."""
    g = Grid()
    # Outer rim implicit. Construct the main rooms.
    # Upper-left: K-in-walls dark room (3-6, 14-18)
    g.shade_dark(3, 14, 6, 18)
    g.room(3, 14, 6, 18); g.door(5, 14, "s")
    g.room(4, 15, 5, 17); g.door(4, 16, "w")  # inner K chamber

    # Top-middle: K corridor
    g.room(8, 17, 11, 19); g.door(9, 17, "s"); g.door(10, 17, "s")
    g.corridor_v(9, 14, 16)
    g.corridor_v(10, 14, 16); g.door(10, 16, "e")

    # Right cluster: D in centred chamber, big room 14-18 x 13-17
    g.room(14, 13, 18, 17); g.door(14, 16, "w"); g.door(18, 14, "e")
    g.room(15, 14, 17, 16); g.door(16, 14, "s")  # inner D chamber

    # Middle corridor y=10-12 with pit (11,11)
    g.corridor_h(11, 7, 13); g.door(11, 11, "s")  # P pit
    g.room(7, 10, 8, 12); g.door(7, 12, "n")
    g.room(12, 9, 13, 12); g.door(13, 12, "n")

    # Middle-east: E elevator at (10,7)
    g.room(10, 6, 11, 8); g.door(10, 8, "n"); g.door(11, 7, "e")
    g.room(12, 6, 14, 8); g.door(14, 8, "n"); g.door(12, 6, "s")

    # West cluster y=5..9 with corridor at y=8
    g.corridor_h(8, 0, 6); g.door(0, 8, "w")
    g.room(0, 9, 2, 10); g.door(1, 9, "s")
    g.room(3, 6, 5, 9); g.door(5, 6, "s"); g.door(3, 9, "n")
    g.room(0, 5, 2, 7); g.door(2, 7, "e")

    # South-west L-zone y=0..5
    g.shade_dark(2, 0, 6, 5)
    g.room(0, 0, 1, 4); g.door(1, 4, "n"); g.door(1, 0, "s")
    g.room(2, 0, 4, 5); g.door(3, 5, "n"); g.door(4, 0, "s")
    g.room(5, 0, 7, 5); g.door(6, 5, "n")
    # Centre-south
    g.room(8, 0, 10, 5); g.door(9, 5, "n"); g.door(8, 0, "s"); g.door(10, 4, "e")
    # Key item room (13,4)
    g.room(11, 0, 13, 5); g.door(13, 4, "e"); g.door(12, 0, "s")
    # East-south
    g.room(14, 0, 16, 5); g.door(15, 5, "n"); g.door(16, 4, "e")
    g.room(17, 0, 19, 5); g.door(17, 5, "n")

    # Far east column 17-19 x 6-12 (vertical corridor)
    g.corridor_v(18, 6, 12); g.door(18, 12, "n")
    g.room(17, 6, 19, 8); g.door(18, 8, "n")
    g.room(17, 10, 19, 12); g.door(18, 10, "s")

    # Top row y=18-19 east cluster
    g.room(12, 17, 13, 19); g.door(12, 17, "s")
    g.room(14, 18, 16, 19); g.door(15, 18, "s")
    g.room(17, 18, 19, 19); g.door(17, 18, "s")

    return g


def build_b3() -> Grid:
    """B3F. Grid of 16 small isolated rooms with many one-way doors. Each
    room is roughly 3x3 cells. Top row has pits (4,19), spinner (9,19), pit
    (16,19). Centre is the U-stair complex around (7-11, 8) and (13,2)."""
    g = Grid()
    # The classic 4x4 grid of rooms each 4 wide x 4 tall.
    # Rooms at positions: (1-4,1-4), (6-9,1-4), (11-14,1-4), (16-19,1-4)
    #                    (1-4,6-9), (6-9,6-9), (11-14,6-9), (16-19,6-9)
    #                    (1-4,11-14), (6-9,11-14), (11-14,11-14), (16-19,11-14)
    #                    (1-4,16-19), (6-9,16-19), (11-14,16-19), (16-19,16-19)
    for row in (1, 6, 11, 16):
        for col in (1, 6, 11, 16):
            g.room(col, row, col + 3, row + 3)
    # Connecting one-way doors (per walkthrough: many warps)
    g.oneway(2, 4, "n"); g.oneway(7, 4, "n"); g.oneway(12, 4, "n"); g.oneway(17, 4, "n")
    g.oneway(2, 9, "n"); g.oneway(7, 9, "n"); g.oneway(12, 9, "n"); g.oneway(17, 9, "n")
    g.oneway(2, 14, "n"); g.oneway(7, 14, "n"); g.oneway(12, 14, "n"); g.oneway(17, 14, "n")
    # Lateral connections
    g.door(4, 2, "e"); g.door(9, 2, "e"); g.door(14, 2, "e")
    g.door(4, 12, "e"); g.door(9, 12, "e"); g.door(14, 12, "e")
    g.door(4, 17, "e"); g.door(9, 17, "e"); g.door(14, 17, "e")
    # Central U-stair complex (7,8) and spinner (13,2)
    g.door(7, 8, "n"); g.door(7, 8, "s")  # stairs_down at (7,8)
    g.door(13, 2, "n"); g.door(13, 2, "w")  # spinner at (13,2)
    # Stairs_up at (0,0) — corner room
    g.room(0, 0, 0, 0); g.door(0, 0, "n")
    return g


def build_b4() -> Grid:
    """B4F. Central elevator complex with key-item room (10,11). Most cells
    outside the central column are solid stone."""
    g = Grid()
    # Solid stone everywhere except the central corridor and the rooms.
    g.block(0, 0, 9, 19)
    g.block(11, 0, 19, 19)
    # Carve out: central vertical corridor x=10 from y=0 to y=19
    for y in range(0, 20):
        g.blocked.discard((10, y))
        cell = g.cells.setdefault((10, y), {})
        # remove all walls on x=10 column
        cell.pop("n", None); cell.pop("s", None); cell.pop("e", None); cell.pop("w", None)
    # Now add outer wall on column x=10
    g.wall(10, 0, "s"); g.wall(10, 19, "n")
    for y in range(0, 20):
        g.wall(10, y, "e"); g.wall(10, y, "w")
    # Stairs_up at (10,18) - room
    g.blocked.discard((10, 18))
    # Elevator at (10,8) - branches east to monster room (12,12)
    # Monster Allocation Center at (12,12) - carve room 11-13 x 11-13
    for y in range(11, 14):
        for x in range(11, 14):
            g.blocked.discard((x, y))
            cell = g.cells.setdefault((x, y), {})
            for side in ("n", "s", "e", "w"):
                cell.pop(side, None)
    g.room(11, 11, 13, 13)
    g.door(11, 12, "w")  # door connecting to corridor
    g.open_passage(10, 12, "e")  # ensure corridor side opens
    # Key item / Blue Ribbon area at (10,11) - in corridor
    # Stairs_down at (17,7) - carve a small east branch
    for y in (7,):
        for x in range(11, 18):
            g.blocked.discard((x, y))
            cell = g.cells.setdefault((x, y), {})
            for side in ("n", "s", "e", "w"):
                cell.pop(side, None)
    g.corridor_h(7, 11, 17)
    g.open_passage(10, 7, "e")
    # West stub corridor to (3,7)
    for x in range(3, 10):
        g.blocked.discard((x, 7))
        cell = g.cells.setdefault((x, 7), {})
        for side in ("n", "s", "e", "w"):
            cell.pop(side, None)
    g.corridor_h(7, 3, 9)
    g.open_passage(10, 7, "w")
    # Outer perimeter wall around traversable area
    return g


def build_b5() -> Grid:
    """B5F. Outer perimeter corridor + central rooms; spinner (4,5) and
    stairs_down (12,7); elevator (9,0)."""
    g = Grid()
    # Outer perimeter corridor
    for x in range(0, 20):
        g.wall(x, 19, "n"); g.wall(x, 0, "s")
    for y in range(0, 20):
        g.wall(0, y, "w"); g.wall(19, y, "e")
    # Some inner rooms
    g.room(2, 2, 6, 6); g.door(4, 2, "s"); g.door(2, 4, "w")
    g.room(2, 8, 6, 12); g.door(2, 10, "w"); g.door(4, 12, "n")
    g.room(2, 14, 6, 17); g.door(4, 14, "s"); g.door(6, 16, "e")
    g.room(8, 2, 12, 6); g.door(10, 6, "n"); g.door(12, 4, "e")
    g.room(8, 8, 11, 11)  # central pit-room area
    g.block(13, 10, 15, 12)  # solid block (Wall zone)
    g.block(13, 14, 15, 16)
    g.room(13, 2, 17, 6); g.door(15, 6, "n")
    g.room(13, 8, 18, 9); g.door(13, 8, "w")
    g.room(8, 14, 12, 17); g.door(10, 14, "s"); g.door(8, 16, "w")
    g.room(16, 14, 18, 17); g.door(16, 16, "w")
    g.room(16, 10, 18, 12); g.door(16, 12, "n")
    return g


def build_b6() -> Grid:
    """B6F. Many pits, mostly-dark lower half, marker scattered."""
    g = Grid()
    # Outer rim
    for x in range(0, 20):
        g.wall(x, 19, "n"); g.wall(x, 0, "s")
    for y in range(0, 20):
        g.wall(0, y, "w"); g.wall(19, y, "e")
    g.shade_dark(2, 6, 17, 8)
    g.shade_dark(2, 3, 5, 5)
    g.shade_dark(15, 1, 17, 5)

    # Top P P D row at y=15
    g.room(4, 15, 5, 17); g.door(5, 15, "s")
    g.room(7, 15, 8, 17); g.door(7, 15, "s")
    g.room(10, 15, 11, 17); g.door(11, 15, "s")  # D = stairs_down at (11,17)
    g.room(13, 15, 16, 17); g.door(14, 15, "s")

    # Mid horizontal corridor at y=13
    g.corridor_h(13, 2, 17)
    # Marker 2 at (13,11)
    g.room(11, 9, 14, 12); g.door(13, 12, "n")

    # Lower west corridors
    g.room(2, 9, 5, 12); g.door(4, 12, "n")
    g.room(7, 9, 9, 12); g.door(8, 12, "n")
    g.room(2, 1, 4, 4); g.door(3, 4, "n")
    g.room(6, 1, 9, 4); g.door(7, 4, "n")
    g.room(10, 1, 13, 4); g.door(11, 4, "n")
    g.room(15, 1, 17, 4); g.door(16, 4, "n")
    return g


def build_b7() -> Grid:
    """B7F. 'Castle' symmetric concentric rooms. Pits clustered lower-left,
    U-stair at (10,10), D-stair at (17,8)."""
    g = Grid()
    # Outer rim
    for x in range(0, 20):
        g.wall(x, 19, "n"); g.wall(x, 0, "s")
    for y in range(0, 20):
        g.wall(0, y, "w"); g.wall(19, y, "e")

    # Concentric rings
    g.room(1, 1, 18, 18)
    g.room(3, 3, 16, 16); g.door(8, 3, "s"); g.door(10, 16, "n")
    g.room(5, 5, 14, 14); g.door(5, 9, "w"); g.door(14, 9, "e")
    g.room(7, 7, 12, 12); g.door(10, 7, "s"); g.door(10, 12, "n")
    # Inner U-stair chamber
    g.room(9, 9, 11, 11); g.door(10, 9, "s")
    # Pit cluster (6,4)-(10,5) and (6,5)-(7,5)
    for px, py in [(6,4),(7,4),(6,5),(7,5),(9,4),(10,4),(9,5),(10,5)]:
        g.door(px, py, "n")  # pit cells need access
    return g


def build_b8() -> Grid:
    """B8F. Densest level — many teleporter pairs (1/1', 2/2', ... 12/12').
    Concentric structure with many small rooms."""
    g = Grid()
    for x in range(0, 20):
        g.wall(x, 19, "n"); g.wall(x, 0, "s")
    for y in range(0, 20):
        g.wall(0, y, "w"); g.wall(19, y, "e")
    # Concentric rings with many doors
    g.room(1, 1, 18, 18); g.door(9, 1, "s"); g.door(10, 18, "n")
    g.room(3, 3, 16, 16); g.door(3, 9, "w"); g.door(16, 9, "e")
    g.room(5, 5, 14, 14); g.door(9, 5, "s"); g.door(10, 14, "n")
    g.room(7, 7, 12, 12); g.door(7, 9, "w"); g.door(12, 9, "e")
    # Inner stairs_up chamber at (8,10)
    g.room(8, 10, 9, 10); g.door(9, 10, "e"); g.door(8, 10, "w")
    # Many pit cells
    for x in range(5, 13):
        for y in (3, 4, 5):
            g.door(x, y, "n")
    return g


def build_b9() -> Grid:
    """B9F. Maze-of-walls; mostly solid with narrow perimeter corridor.
    Chute at (8,2) to Werdna's domain. Elevator at (10,0)."""
    g = Grid()
    # Solid centre block
    g.block(2, 3, 17, 17)
    # Perimeter corridor (row y=0..2, y=18, columns x=0..1, x=18..19)
    for x in range(0, 20):
        g.wall(x, 19, "n"); g.wall(x, 0, "s")
    for y in range(0, 20):
        g.wall(0, y, "w"); g.wall(19, y, "e")
    # Carve out perimeter corridor cells
    for x in range(0, 20):
        for y in (0, 1, 2):
            g.blocked.discard((x, y))
        for y in (18,):
            g.blocked.discard((x, y))
    for y in range(0, 20):
        for x in (0, 1):
            g.blocked.discard((x, y))
        for x in (18, 19):
            g.blocked.discard((x, y))
    # The chute is at (8,2) - special door access
    g.door(8, 2, "n")
    # Elevator at (10,0)
    g.door(10, 0, "n")
    return g


def build_b10() -> Grid:
    """B10F Werdna's Domain. Maze layout with numbered teleporters; chute
    landing at (6,0); boss room ~(9,9); elevator at (10,0)."""
    g = Grid()
    for x in range(0, 20):
        g.wall(x, 19, "n"); g.wall(x, 0, "s")
    for y in range(0, 20):
        g.wall(0, y, "w"); g.wall(19, y, "e")
    # Several solid blocks scattered (dark grey on PDF)
    g.block(3, 5, 4, 8)
    g.block(7, 5, 8, 8)
    g.block(11, 5, 12, 8)
    g.block(15, 5, 16, 8)
    g.block(5, 15, 7, 17)
    g.block(11, 15, 13, 17)
    g.block(3, 11, 5, 13)
    g.block(8, 11, 10, 13)
    g.block(14, 11, 16, 13)
    # Central boss chamber
    g.room(8, 8, 10, 10); g.door(9, 10, "n"); g.door(9, 8, "s")
    # Corridors carved between blocks
    g.corridor_h(10, 1, 18)
    g.corridor_h(4, 1, 18)
    g.corridor_h(14, 1, 18)
    g.corridor_v(2, 1, 18)
    g.corridor_v(18, 1, 18)
    return g


BUILDERS = {
    1: build_b1, 2: build_b2, 3: build_b3, 4: build_b4, 5: build_b5,
    6: build_b6, 7: build_b7, 8: build_b8, 9: build_b9, 10: build_b10,
}


def merge_floor(floor: dict, grid: Grid) -> dict:
    """Merge generated wall cells with the existing feature cells, preserving
    feature/marker/label/dark/encounter info."""
    existing = {(c["x"], c["y"]): c for c in floor.get("cells", [])}
    generated = {(c["x"], c["y"]): c for c in grid.to_cells()}

    merged: dict[tuple[int, int], dict] = {}
    # Start with generated
    for k, v in generated.items():
        merged[k] = dict(v)
    # Overlay existing (features win, walls preserved from generated)
    for k, ev in existing.items():
        m = merged.setdefault(k, {"x": k[0], "y": k[1]})
        for key, val in ev.items():
            if key in ("x", "y"):
                continue
            # Walls from existing only override if explicitly set
            if key in ("n", "s", "e", "w") and key in m:
                # prefer existing if it differs (manual overrides)
                m[key] = val
            else:
                m[key] = val

    # Sort for stable output
    cells = sorted(merged.values(), key=lambda c: (c["y"], c["x"]))
    floor = dict(floor)
    floor["cells"] = cells
    return floor


def main() -> int:
    data = json.loads(SRC.read_text())
    for floor in data["floors"]:
        level = floor["level"]
        builder = BUILDERS.get(level)
        if builder is None:
            continue
        grid = builder()
        merged = merge_floor(floor, grid)
        floor.clear()
        floor.update(merged)
        print(f"  ✓ B{level:02d}F: {len(merged['cells'])} cells")

    SRC.write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n")
    print(f"\nWrote {SRC}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
