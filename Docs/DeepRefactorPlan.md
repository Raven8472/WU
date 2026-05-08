# WU Deep Refactor Plan

Last updated: 2026-05-07

## Current State

The project is in the middle of an asset organization pass. Most non-character marketplace packs have been moved into:

```text
Content/AssetPacks/
```

The following root-level asset folders were cleaned out or moved:

- `Brushify`
- `CraftResourcesIcons`
- `MAEBeechForest`
- `MAEOakForest`
- `MurdocMaterials`
- `QuadrapedCreatures`
- `School_Of_Magic`
- `StonePineForest`

Collision leftovers from the partial move were preserved, not deleted:

```text
Saved/AssetPackMoveLeftovers/
Saved/AssetPackMoveConflicts.txt
```

`Content/StylizedCharacter` is intentionally still at root. Do not move it casually. The runtime character and character creator preview currently load assets from hardcoded `/Game/StylizedCharacter/...` paths in C++.

## Immediate Post-Move Cleanup

1. Open the project in Unreal.
2. Let the asset registry settle.
3. Run "Fix Up Redirectors" on `Content` and especially `Content/AssetPacks`.
4. Check whether levels that used landscape/building/forest packs are broken.
5. If they are broken, accept the breakage and rebuild those areas rather than spending a lot of time recovering early prototype work.
6. Keep `StylizedCharacter` at root until we are ready to move it and update C++ references in the same pass.

## Refactor Goals

The main goal is to turn the current prototype-heavy gameplay code into smaller systems that can survive MMO-scale growth.

Longer-term system direction is tracked in:

```text
Docs/FutureSystemsPathingPlan.md
```

That plan reserves room for Clubs as the guild-style social system and House Factions as the story/flavor identity system.

The two highest-pressure classes are:

- `Source/WU/WUCharacter.h`
- `Source/WU/WUCharacter.cpp`
- `Source/WU/WUPlayerController.h`
- `Source/WU/WUPlayerController.cpp`

`AWUCharacter` currently owns too much:

- movement input
- camera and mouse steering
- combat and damage
- health, magic, regeneration
- death, corpse, release, revive
- inventory and equipment state
- modular character appearance
- hardcoded asset paths
- zone state

`AWUPlayerController` currently owns too much:

- input mapping
- HUD creation and layout
- targeting
- chat
- inventory panel toggling
- character panel toggling
- character creator preview
- selected backend character context
- periodic location persistence

## Phase 0: Stabilize Before Refactor

Before moving gameplay code:

1. Build the project once in its current state.
2. Launch PIE with the current character.
3. Verify:
   - login or dev-login still works
   - character select still works
   - character creator preview still renders
   - spawned gameplay character appears
   - basic movement works
   - inventory UI opens
   - character panel opens
   - targeting still works
   - death/release path still works if convenient

Any broken marketplace scenery is acceptable for now. Broken character flow is not.

## Phase 1: Asset Path Indirection

Before moving `StylizedCharacter`, remove direct hardcoded path sprawl.

Status as of 2026-05-07: first slice complete. `AWUCharacter` and `AWUCharacterCreatorPreviewActor` now share `FWUCharacterAssetPaths` for live character and character creator preview mesh, material, and animation paths. `Content/StylizedCharacter` has not been moved.

Recommended first code refactor:

Create a small character asset path helper or settings object that owns the `/Game/StylizedCharacter` prefix and path builders.

Candidate names:

- `FWUCharacterAssetPaths`
- `UWUCharacterAppearanceAssetLibrary`
- `UWUCharacterAssetSettings`

Minimum outcome:

- `AWUCharacter` no longer repeats every path inline.
- `AWUCharacterCreatorPreviewActor` shares the same path source.
- Moving `StylizedCharacter` later becomes a one-file or config/data-asset update.

This should happen before extracting full character components, because it reduces the riskiest duplication first.

## Phase 2: Split AWUCharacter

Extract behavior in small, compiling slices. Do not attempt one massive split.

Suggested order:

1. Appearance
   - Move modular mesh component setup.
   - Move mesh/material/animation asset lookup.
   - Move appearance application and equipment visual application.
   - This directly prepares us for moving `StylizedCharacter` later.

2. Inventory and Equipment
   - Move bag/equipment arrays and equip/unequip logic.
   - Keep replication behavior explicit.
   - Preserve current Blueprint API wrappers on `AWUCharacter` while forwarding internally.

3. Progression and Vitals
   - Move level, XP, primary stats, derived stats, health, magic, regeneration.
   - Keep server authority clear.

4. Combat and Death
   - Move attack trace, damage application, combat timeout.
   - Move death/release/revive state after vitals are stable.

5. Zone State
   - Move current zone id, display name, map region, graveyard tag.
   - Keep `AWUZoneVolume` integration simple.

6. Locomotion and Camera
   - Only split this after the above systems are calmer.
   - Mouse steering, backpedal, turn-in-place, and camera zoom are tightly coupled to pawn behavior.

## Phase 3: Split AWUPlayerController

After `AWUCharacter` is calmer, split controller concerns.

Suggested order:

1. Targeting helper/component
   - current target replication
   - click target
   - tab target
   - target validation
   - target destroyed handling

2. HUD/window manager
   - player frame
   - target frame
   - XP bar
   - zone name
   - inventory widget
   - character panel
   - character creator widget

3. Chat controller/helper
   - open/close input
   - sanitize message
   - cooldown
   - server/client delivery

4. Character creator controller/helper
   - preview actor lifecycle
   - preview appearance application
   - create request submission

5. Session context helper
   - selected character application
   - spawn identity
   - location save timer

## Phase 4: Backend Client Cleanup

`UWUClientSessionSubsystem` is useful, but it is becoming a mixed HTTP client, JSON parser, auth state holder, and character cache.

Later split:

- request construction
- response parsing
- session state/cache
- public Blueprint-facing subsystem

This is not the first priority unless backend iteration starts slowing us down.

## Phase 5: Editor Tool Cleanup

The editor Python scripts now have repeated Unreal helper code.

Create:

```text
Tools/Editor/wu_editor_utils.py
```

Move shared helpers there:

- logging
- asset path helpers
- class name helpers
- material parameter reads
- dependency reads
- safe asset save routines

Then update:

- `audit_landscape_materials.py`
- `audit_building_materials.py`
- `audit_external_building_actors.py`
- `inspect_church_material_sources.py`
- `stylize_building_materials.py`
- `apply_church_textured_lite.py`

## Recommended First Work Session

When returning, do this first:

1. Open Unreal and fix redirectors.
2. Confirm `StylizedCharacter` still powers character creation and gameplay.
3. Build C++.
4. Start Phase 1: centralize character asset paths.

The first real code change should be small:

- Create one shared path source for stylized character assets.
- Update `AWUCharacter` and `AWUCharacterCreatorPreviewActor` to use it.
- Do not move `StylizedCharacter` yet.
- Build and smoke test character creator plus gameplay character.

After that, moving `StylizedCharacter` to `Content/AssetPacks/Common/StylizedCharacter` or similar becomes much safer.

## Future Asset Pack Import Workflow

Marketplace/Fab "Add to Project" usually lands assets at the path chosen by the pack author, often `/Game/PackName`. It generally does not let us dictate a clean landing folder per project.

Preferred workflow:

1. Add marketplace packs into a blank staging Unreal project.
2. Inspect the pack folder structure.
3. Migrate or move the pack into this project under:

```text
/Game/AssetPacks/<PackName>
```

4. Run "Fix Up Redirectors" immediately.
5. Keep project-owned assets outside `AssetPacks`, for example:

```text
/Game/World
/Game/UI
/Game/Blueprints
/Game/Characters
```

Raw imports are different: for FBX, texture, and source files imported manually, choose the target folder in the Content Browser before importing.

## Rule Of Thumb

Asset packs are vendor material. Keep them under `AssetPacks`.

Game-owned assets are WU material. Keep them in stable project folders.

Character assets currently sit in the middle because game code depends on them directly. Treat `StylizedCharacter` as live infrastructure until the C++ path indirection is finished.
