# ASCII Dungeon Crawler

A turn-based dungeon crawler built in pure C++17, rendered entirely in the Windows console using ASCII characters.

![Platform](https://img.shields.io/badge/platform-Windows-blue)
![Language](https://img.shields.io/badge/language-C%2B%2B17-orange)
![Build](https://img.shields.io/badge/build-Visual%20Studio%202026-purple)

---

## Overview

Each dungeon floor is procedurally generated at the start of every level and remains static until the player descends. The player explores using Manhattan movement, fights enemies in turn-based combat, loots chests, trades with merchants, manages an inventory and equipment loadout, and reaches the exit to descend deeper.

The game is session-based — no save system. You die, you start over.

---

## Features

- Procedural dungeon generation via room placement and L-shaped Manhattan corridors
- Turn-based combat with damage variance and critical hits
- Full inventory system (capacity upgradeable via merchant)
- Equipment system with 7 slots: Head, Chest, Arms, Legs, Boots, Weapon, Shield
- Persistent chest system — loot what you want, leave the rest
- Merchant with randomised stock per floor — buy, sell, upgrade inventory
- Floor progression with enemy scaling — enemies get tougher the deeper you go
- Equipment stat bonuses applied dynamically to player attack, defense, and HP
- ASCII HUD panel showing player stats, equipment, inventory, and context-sensitive controls

---

## Controls

| Input | Action |
|-------|--------|
| Arrow Keys | Move / Navigate menus |
| Space | Interact / Attack / Confirm |
| Tab | Toggle Inventory mode |
| Y / N | Confirm prompts |
| Esc | Back / Quit to menu |

---

## Building

**Requirements:**
- Windows
- Visual Studio 2022 (or 2019 with C++17 support)
- No external dependencies — standard library only

**Steps:**
1. Clone the repo
2. Open `ASCIIDungeonCrawler.sln` in Visual Studio
3. Build and run (Debug or Release)

**Console setup:**
Right-click the console title bar → Properties → Layout and set:
- Screen Buffer Size: 120 wide, 42 tall
- Window Size: 120 wide, 42 tall

Windows remembers this per executable.

---

## Architecture

The project is structured as a clean multi-file C++ codebase with a clear separation of concerns.

```
src/
├── Types.h              # Shared constants and enums
├── Tile.h               # Tile struct and TileType enum
├── Entity.h/cpp         # Base class for all world entities
├── Enemy.h/cpp          # Derived enemy class, tier system, floor scaling
├── Merchant.h/cpp       # Derived merchant class, stock generation, pricing
├── Player.h             # Player stats, inventory, and equipment
├── Item.h               # Item struct, ItemType and EquipSlot enums
├── InventorySystem.h/cpp# Flat item list, scroll state, capacity management
├── EquipmentSystem.h/cpp# 7 equipment slots, dynamic stat bonus computation
├── ItemDatabase.h/cpp   # Singleton item registry, weighted random loot
├── Dungeon.h/cpp        # Grid, room/corridor generation, entity placement
├── Renderer.h/cpp       # All rendering via WriteConsoleOutputCharacterA
├── Input.h/cpp          # _getch() abstraction, Action enum
├── CombatSystem.h/cpp   # Turn-based combat, damage formula, crits
├── Game.h/cpp           # State machine, main loop, all interaction handlers
├── MainMenu.h/cpp       # Main menu, returns bool to main loop
└── main.cpp             # App entry point
```

### Game States

```
Exploring → Combat
          → ChestLoot
          → InventoryAction
          → MerchantMenu (TopMenu → Buy / Sell)
          → ExitPrompt
          → QuitPrompt
          → GameOver
```

### Key Design Decisions

**No A\* pathfinding** — corridors use L-shaped Manhattan connections. Rooms are connected in chain order (room 0 → 1 → 2 → ... → N), which guarantees full connectivity by construction — the same logical guarantee as a linked list. At 80×30 console scale, this is sufficient and adds zero complexity.

**WriteConsoleOutputCharacterA for rendering** — characters are written at absolute (col, row) coordinates with no cursor movement. This eliminates flicker entirely without double buffering or clearing the screen.

**unique\_ptr for entity ownership** — entities are stored as `vector<unique_ptr<Entity>>` in `Dungeon`. Polymorphic dispatch works correctly, memory is cleaned up automatically on floor regeneration, and there is no manual `delete` anywhere in the codebase.

**unordered\_map for chest and entity lookup** — both the renderer and game logic look up entities by position. Chests are stored as `unordered_map<int, vector<Item>>` keyed by `y * MAP_WIDTH + x`. O(1) lookup per tile, consistent across both systems.

**Computed stats** — `Player::attack()`, `Player::defense()`, and `Player::maxHP()` are methods, not fields. Equipment bonuses are applied dynamically at call time with no need to track dirty state or recalculate on equip/unequip.

**Floor scaling at generation time** — enemy stats are scaled when the dungeon generates, not at combat time. Zero runtime overhead during gameplay.

**NOMINMAX before windows.h** — defined at the top of every `.cpp` file that includes Windows headers, preventing the `min`/`max` macros from stomping `std::min` and `std::max`.

---

## Symbols

| Symbol | Meaning |
|--------|---------|
| `@` | Player |
| `!` | Basic enemy (Grunt) |
| `?` | Agile enemy (Trickster) |
| `+` | Heavy enemy (Brute) |
| `~` | Merchant |
| `C` | Chest (contains items) |
| `c` | Chest (looted) |
| `>` | Exit / Descend |
| `#` | Void / Inaccessible |
| `.` | Floor |
| `-` | Horizontal wall |
| `\|` | Vertical wall |

---

## Memory

Debug build: ~1MB  
The grid is 80×30 = 2400 tiles at ~8 bytes each (~19KB). The remainder is entity data, item strings, and MSVC debug instrumentation overhead. Release build is substantially smaller.

---

## Planned / Future

- Enemy patrol movement
- Fog of war
- SFML rendering layer + BSP dungeon generation
- Pseudo-3D raycasting renderer (Wolfenstein-style)

---

## License

MIT — free to read, learn from, and use.
