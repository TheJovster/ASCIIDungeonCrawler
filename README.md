# ASCII Dungeon Crawler

A turn-based dungeon crawler built in pure C++17. Two versions exist across two branches — a dependency-free console build and a full pseudo-3D SFML renderer.

![Platform](https://img.shields.io/badge/platform-Windows-blue)
![Language](https://img.shields.io/badge/language-C%2B%2B17-orange)
![Build](https://img.shields.io/badge/build-Visual%20Studio%2026-purple)

---

## Branches

| Branch | Description |
|--------|-------------|
| `ascii` | Complete console version. Zero external dependencies. Pure C++ standard library only. |
| `bsp-rendered` | Full SFML version. Pseudo-3D raycasting renderer, textures, spatial audio, smooth movement. Active development. |
| `main` | Merge target. Updated when `bsp-rendered` is stable. |

---

## Overview

Each dungeon floor is procedurally generated on descent and remains static until the player moves on. The player explores first-person using grid-locked movement, fights enemies in turn-based combat, loots chests, trades with merchants, manages an inventory and equipment loadout, and reaches the exit to descend deeper.

The game is session-based. No save system. You die, you start over.

---

## Features

### Core Systems
- Procedural dungeon generation — room placement with L-shaped Manhattan corridors
- Turn-based combat with diminishing returns defense formula
- Full inventory system — capacity upgradeable via merchant
- Equipment system with 8 slots: Head, Chest, Arms, Legs, Boots, Weapon, Shield, Torch
- Procedural item generation — 8 material tiers (Leather to Adamantium), weighted by floor depth
- Merchant economy — randomised stock per floor, buy, sell, inventory upgrade
- Floor progression — enemy stats and loot quality scale with depth
- Torch system — equippable item, burns one charge per step, darkness without it
- Enemy patrol movement — enemies roam between turns

### Renderer (bsp-rendered)
- Doom/Wolfenstein-style pseudo-3D raycasting renderer
- Per-pixel textured walls, floor, and ceiling
- Billboard sprites for enemies, merchants, chests, and exit
- Z-buffer occlusion — sprites correctly occlude behind walls
- Dynamic torch lighting with quadratic falloff and flicker
- Smooth movement and rotation lerp — visual position independent of grid position
- 2D minimap overlay with all entities, colored by type
- HUD panel — stats, equipment, inventory, combat log, context controls

### Audio
- Unified AudioManager singleton — menu music, game music, game over track
- SFX hooks for attack, damage, chest, merchant interactions

---

## Controls (bsp-rendered)

| Input | Action |
|-------|--------|
| W / S | Move forward / backward |
| A / D | Strafe left / right |
| Q / E | Rotate left / right |
| Space | Interact — face target first |
| Tab | Toggle inventory |
| Y / N | Confirm / deny prompts |
| Esc | Quit menu |

**Interaction:** Face an enemy, chest, merchant, or exit and press Space to interact. You must be on an adjacent tile and facing the target.

---

## Controls (ascii)

| Input | Action |
|-------|--------|
| Arrow Keys | Move / Navigate menus |
| Space | Interact / Attack / Confirm |
| Tab | Toggle inventory |
| Y / N | Confirm / deny prompts |
| Esc | Back / Quit |

---

## Building

### bsp-rendered (SFML)

**Requirements:**
- Windows
- Visual Studio 2022 with C++17
- SFML 2.6.2 (included in repo under SFML-2.6.2/)

**Steps:**
1. Clone the repo and switch to bsp-rendered
2. Open ASCIIDungeonCrawler.sln
3. Build x64 Release or Debug
4. SFML DLLs are expected in the output directory alongside the executable

**Assets** — place in assets/ relative to the executable:

    assets/
    MenuMusic.ogg
    GameMusic.ogg
    GameOverMusic.ogg
    texture_wall.png
    texture_floor.png
    texture_ceiling.png
    texture_merchant.png
    texture_chest_closed.png
    texture_chest_opened.png
    texture_doors.png
    sfx_*.ogg  (optional — fail silently if missing)

Audio must be .ogg, .wav, or .flac. MP3 is not supported by SFML.

### ascii (Console)

**Requirements:**
- Windows
- Visual Studio 2022 with C++17
- No external dependencies

**Steps:**
1. Clone the repo and switch to ascii
2. Open ASCIIDungeonCrawler.sln
3. Build and run

---

## Architecture

Clean multi-file C++ with strict separation of concerns. Game logic, rendering, input, and audio are fully decoupled.

    Types.h                  Shared constants and enums
    Tile.h                   Tile struct and TileType enum
    Entity.h/cpp             Base class for all world entities
    Enemy.h/cpp              Derived enemy, tier system, floor scaling
    Merchant.h/cpp           Derived merchant, stock generation, pricing
    Player.h                 Player stats, inventory, equipment, visual position
    Item.h                   Item struct, ItemType and EquipSlot enums
    InventorySystem.h/cpp    Item list, scroll state, capacity management
    EquipmentSystem.h/cpp    8 equipment slots, dynamic stat computation
    ItemDatabase.h/cpp       Singleton item registry, weighted procedural loot
    Dungeon.h/cpp            Grid, generation, entity and chest placement
    CombatSystem.h/cpp       Turn-based combat, damage formula
    AudioManager.h/cpp       Singleton audio — music tracks and SFX
    RaycastRenderer.h/cpp    Pseudo-3D raycaster, sprites, minimap [bsp-rendered]
    Renderer.h/cpp           HUD panel rendering (both branches)
    Input.h/cpp              sf::Event abstraction, Action enum
    Game.h/cpp               State machine, main loop, interaction handlers
    MainMenu.h/cpp           Main menu
    HowToPlay.h/cpp          4-page tutorial screen [bsp-rendered]
    main.cpp                 Entry point, window, game loop

### Game States

    Exploring -> Combat
              -> ChestLoot
              -> InventoryAction
              -> MerchantMenu (TopMenu -> Buy / Sell / Upgrade)
              -> ExitPrompt
              -> QuitPrompt
              -> GameOver

### Key Design Decisions

**No A* pathfinding** — corridors use L-shaped Manhattan connections in chain order. Full connectivity guaranteed by construction. Zero complexity overhead.

**Integer grid position + float visual position** — all game logic uses int x, y. The renderer reads float visualX, visualY which lerps toward grid position each frame. Smooth movement with zero impact on game systems.

**Raycasting with single pixel buffer** — floor, ceiling, walls, and sprites all write into one sf::Image pixel buffer per frame. One texture.update() and one window.draw() covers the entire 3D view. Sprites use a Z-buffer for correct occlusion.

**Torch as equipment** — torches are ItemType::Equipment in the Torch slot. Each step decrements charges. When charges reach zero the slot clears. Darkness is genuine resource pressure, not just atmosphere.

**unique_ptr for entity ownership** — entities stored as vector<unique_ptr<Entity>> in Dungeon. Polymorphic dispatch, automatic cleanup on floor regen, no manual delete.

**Computed stats** — Player::attack(), Player::defense(), Player::maxHP() are methods. Equipment bonuses applied at call time. No dirty state, no recalculation on equip/unequip.

**Singleton AudioManager** — owns all sf::Music and sf::Sound state. playMusic(MusicTrack::Game) handles stop/start/loop automatically.

**/SUBSYSTEM:WINDOWS via pragma** — #pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup") in main.cpp. Suppresses the console window without touching .vcxproj. Version-controlled, no machine-specific setup.

---

## Item Tiers

| Material | Bonus | Min Floor | Value Multiplier |
|----------|-------|-----------|-----------------|
| Leather | +1 | 1 | 1x |
| Copper | +2 | 1 | 2x |
| Iron | +3 | 2 | 3x |
| Steel | +5 | 3 | 5x |
| Silver | +7 | 4 | 8x |
| Platinum | +10 | 5 | 12x |
| Black Iron | +13 | 6 | 16x |
| Adamantium | +17 | 8 | 22x |

---

## Performance (bsp-rendered, Release)

- Memory: ~50MB
- CPU: ~1.6% idle
- Tested at 1280x720, 30fps cap

---

## License

MIT — free to read, learn from, and use.
