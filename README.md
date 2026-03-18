# ASCII Dungeon Crawler

A turn-based dungeon crawler built in C++17 with a Doom/Wolfenstein-style pseudo-3D raycasting renderer. Two versions exist across two branches — a dependency-free console build and a full SFML renderer.

![Platform](https://img.shields.io/badge/platform-Windows-blue)
![Language](https://img.shields.io/badge/language-C%2B%2B17-orange)
![Build](https://img.shields.io/badge/build-Visual%20Studio%202026-purple)

---

## Branches

| Branch | Description |
|--------|-------------|
| `ascii` | Complete console version. Zero external dependencies. Pure C++ standard library only. |
| `bsp-rendered` | Full SFML version. Pseudo-3D raycasting renderer, textures, spatial audio, animated sprites, smooth movement. Active development. |
| `main` | Merge target. Updated from `bsp-rendered` when stable. |

---

## Overview

Each dungeon floor is procedurally generated on descent and remains static until the player moves on. The player explores first-person using grid-locked movement, fights enemies in turn-based combat, loots chests, trades with merchants, manages an inventory and equipment loadout, rests to recover health, and reaches the exit to descend deeper.

The game is session-based. No save system. You die, you start over.

---

## Features

### Core Systems
- Procedural dungeon generation — room placement with L-shaped Manhattan corridors
- Turn-based combat with diminishing returns defense formula
- Ambush system — enemy-initiated combat gives the player a fight or flee choice before engagement
- Full inventory system — capacity upgradeable via merchant
- Equipment system with 8 slots: Head, Chest, Arms, Legs, Boots, Weapon, Shield, Torch
- Procedural item generation — 8 material tiers (Leather to Adamantium), weighted by floor depth
- Merchant economy — randomised stock per floor, buy, sell, inventory upgrade
- Floor progression — enemy stats and loot quality scale with depth
- Torch system — equippable item, burns one charge per step, darkness without it
- Enemy patrol movement — enemies roam between turns
- Rest system — rest to heal (6 HP/hour), wait to advance time, interrupted by nearby enemies
- Persistent corpse system — defeated enemies leave a death pose in the world until the next floor

### Renderer (bsp-rendered)
- Doom/Wolfenstein-style pseudo-3D raycasting renderer (DDA algorithm)
- Per-pixel textured walls, floor, and ceiling
- Billboard sprites for enemies, merchants, chests, and exit
- Z-buffer occlusion — sprites correctly occlude behind walls
- Dynamic torch lighting with quadratic falloff and flicker
- Smooth movement and rotation lerp — visual position independent of grid position
- 2D minimap overlay with fog of war — visited tiles persist, visible tiles update per move
- HUD panel — stats, equipment, inventory, combat log, context controls
- Red vignette on player hit, white flash on critical hit, black fade on rest/wait
- Borderless fullscreen at native desktop resolution — all layout derived at runtime

### Animator System
- Clip-based animation — each entity owns an Animator with registered AnimationClips
- Non-looping clips auto-return to a designated idle state on completion
- `forceState()` bypasses guards for interrupt-critical transitions (e.g. death mid-hit)
- Trickster enemy: idle, attack, hit, die, dead animations at 12fps
- All missing clips fail silently — no crashes on missing assets

### Audio
- Unified AudioManager singleton — menu music, gameplay music, game over track
- SFX support for attack, damage, death, chest, merchant interactions
- Music transitions handled automatically on state change

---

## Controls (bsp-rendered)

| Input | Action |
|-------|--------|
| W / S | Move forward / backward |
| A / D | Strafe left / right |
| Q / E | Rotate left / right |
| Space | Interact — face target first |
| R | Open rest menu |
| Tab | Toggle inventory |
| Y / N | Confirm / deny prompts |
| Esc | Back / quit menu |
| Left Stick | Move (Xbox controller) |
| Right Stick | Rotate (Xbox controller) |
| A Button | Confirm (Xbox controller) |

**Interaction:** Face an enemy, chest, merchant, or exit and press Space to interact. You must be on an adjacent tile and facing the target.

---

## Controls (ascii)

| Input | Action |
|-------|--------|
| WASD | Move / navigate menus |
| Arrow Keys | Navigate main menu |
| Space | Interact / attack / confirm |
| Tab | Toggle inventory |
| Y / N | Confirm / deny prompts |
| Esc | Back / quit |

---

## Building

### bsp-rendered (SFML)

**Requirements:**
- Windows
- Visual Studio 2022 with C++17
- SFML 2.6.2 (included in repo under `SFML-2.6.2/`)

**Steps:**
1. Clone the repo and switch to `bsp-rendered`
2. Open `ASCIIDungeonCrawler.sln`
3. Build x64 Release or Debug
4. SFML DLLs must be in the output directory alongside the executable

**Assets** — place in `assets/` relative to the executable:

```
assets/
  button.png
  MenuMusic.ogg
  GameplayMusic.ogg
  GameOverMusic.ogg
  texture_wall.png          (1024x1024)
  texture_floor.png         (1024x1024)
  texture_ceiling.png       (1024x1024)
  texture_merchant.png
  texture_chest_closed.png
  texture_chest_opened.png
  texture_doors.png
  enemy_grunt.png
  enemy_brute.png
  sfx_attack.ogg            (optional — fails silently if missing)
  sfx_takedamage.ogg
  sfx_enemyDeath.ogg
  sfx_openchest.ogg
  sfx_closechest.ogg
  sfx_buyitem.ogg
  sfx_sellitem.ogg
  sfx_select.ogg
  animations/
    trickster/
      enemy_trickster_idle_1.png ... enemy_trickster_idle_12.png
      enemy_trickster_attack_1.png ... enemy_trickster_attack_12.png
      enemy_trickster_hit_1.png ... enemy_trickster_hit_12.png
      enemy_trickster_death_1.png ... enemy_trickster_death_12.png
      enemy_trickster_dead.png
    merchant/
      merchant_idle_1.png ... merchant_idle_12.png
```

Audio must be `.ogg`, `.wav`, or `.flac`. MP3 is not supported by SFML.

Wall, floor, and ceiling textures must be 1024x1024. The raycaster samples them at `TEX_SIZE = 1024`.

### ascii (Console)

**Requirements:**
- Windows
- Visual Studio 2022 with C++17
- No external dependencies

**Steps:**
1. Clone the repo and switch to `ascii`
2. Open `ASCIIDungeonCrawler.sln`
3. Build and run

---

## Architecture

Clean multi-file C++ with strict separation of concerns. Game logic, rendering, input, and audio are fully decoupled.

```
Types.h                   Shared constants and enums
Tile.h                    Tile struct and TileType enum
Entity.h                  Base class for all world entities. Owns Animator.
Enemy.h/cpp               Derived enemy — type system, per-type animation registration
Merchant.h/cpp            Derived merchant — stock generation, pricing, idle animation
EntityFactory.h/cpp       Static factory — centralised entity construction
Player.h                  Player stats, inventory, equipment, visual position
Item.h                    Item struct, ItemType and EquipSlot enums
InventorySystem.h/cpp     Item list, scroll state, capacity management
EquipmentSystem.h/cpp     8 equipment slots, dynamic stat computation
ItemDatabase.h/cpp        Singleton item registry, weighted procedural loot
Dungeon.h/cpp             Grid, generation, entity and chest placement
CombatSystem.h/cpp        Turn-based combat, damage formula, animator triggers
AudioManager.h/cpp        Singleton audio — music tracks and SFX
Animator.h/cpp            Clip-based animation state machine
GameTime.h/cpp            Fantasy clock and calendar, advances per move/turn/hour
RaycastRenderer.h/cpp     Pseudo-3D raycaster, sprites, minimap, overlays
Renderer.h/cpp            HUD panel rendering
Input.h/cpp               sf::Event abstraction, Action enum
Game.h                    State machine definition, member declarations
Game.cpp                  Constructor, run loop, spawnPlayer, updateVisibility
Game_Combat.cpp           Combat state handlers, resolveCombatTurn, endCombat
Game_Exploration.cpp      Exploration handler, enemy patrol, adjacency queries
Game_Interaction.cpp      Chest, merchant, inventory, exit, quit handlers
Game_Rest.cpp             Rest menu, wait select, resting, waiting, enemiesNearby
Button.h/cpp              UI button — texture, label, hover/press states
MainMenu.h/cpp            Main menu with mouse and keyboard navigation
HowToPlay.h/cpp           4-page tutorial screen
ASCIIDungeonCrawler.cpp   Entry point, window, loading screen, game loop
```

### Game States

```
Exploring -> Combat (player or enemy initiated)
             -> Ambush (enemy initiated — fight or flee prompt)
          -> ChestLoot
          -> InventoryAction
          -> MerchantMenu (TopMenu -> Buy / Sell / Upgrade)
          -> ExitPrompt
          -> QuitPrompt
          -> RestMenu -> RestWaitSelect
                      -> Resting
                      -> Waiting
          -> GameOver
```

### Key Design Decisions

**No A* pathfinding** — corridors use L-shaped Manhattan connections in chain order. Full connectivity guaranteed by construction. Zero complexity overhead.

**Integer grid position + float visual position** — all game logic uses `int x, y`. The renderer reads `float visualX, visualY` which lerps toward grid position each frame. Smooth movement with zero impact on game systems. Entities share the same pattern.

**Raycasting with single pixel buffer** — floor, ceiling, and walls all write into one `sf::Image` pixel buffer per frame. One `texture.update()` and one `window.draw()` covers the entire 3D view. Sprites use a separate `sf::VertexArray` quad path with Z-buffer occlusion.

**Torch as equipment** — torches are `ItemType::Equipment` in the Torch slot. Each step decrements charges. When charges reach zero the slot clears. Light radius in the raycaster and minimap is derived from torch state. Darkness is genuine resource pressure.

**Animator owns no textures** — the Animator stores asset path strings as frame data. `RaycastRenderer` owns the texture cache. `getTextureName()` returns the current frame path, the renderer looks it up. Clean separation — animation logic never touches GPU resources.

**EntityFactory** — all entity construction goes through `EntityFactory::createEnemy()` and `createMerchant()`. `Dungeon` calls the factory, never constructs directly. Adding a new entity type means adding a factory method, nothing else changes.

**Corpse system** — entities are not removed from the world on death. `setCorpse(true)` marks them as post-death. Corpses are skipped by all game logic (patrol, combat, adjacency) but rendered by the raycaster and ticked by the animator. The death animation plays, settles on the dead pose, and persists until `nextFloor()` clears all entities.

**`unique_ptr` for entity ownership** — entities stored as `vector<unique_ptr<Entity>>` in Dungeon. Polymorphic dispatch, automatic cleanup on floor regen, no manual delete.

**Computed stats** — `Player::attack()`, `Player::defense()`, `Player::maxHP()` are methods. Equipment bonuses applied at call time. No dirty state, no recalculation on equip/unequip.

**Singleton AudioManager** — owns all `sf::Music` and `sf::Sound` state. `playMusic(MusicTrack::Game)` handles stop/start/loop automatically. SFX buffers loaded at construction, fail silently if files are missing.

**Resolution-agnostic layout** — `SCREEN_WIDTH` and `SCREEN_HEIGHT` constants removed. All renderer dimensions derived from `sf::RenderWindow::getSize()` at construction. Runs at any desktop resolution.

**`/SUBSYSTEM:WINDOWS` via pragma** — `#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")` in `ASCIIDungeonCrawler.cpp`. Suppresses the console window without touching `.vcxproj`. Version-controlled, no machine-specific setup.

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

- Borderless fullscreen at native desktop resolution
- 60fps cap via `window.setFramerateLimit(60)`
- Memory: ~75MB VRAM (GPU texture cache), ~10MB total assets
- CPU: ~1-2% idle, stable under load
- Tested on Windows 10/11 x64

---

## License

MIT — free to read, learn from, and use.
