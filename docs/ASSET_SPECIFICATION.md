# Tornia-guld — Asset Specification

> **Status:** Initial production draft — Version 0.1  
> **Project:** Tornia-guld  
> **Companion documents:** [GDD](GDD.md), [Technical Design](TECHNICAL_DESIGN.md), [Software Architecture](SOFTWARE_ARCHITECTURE.md).  
>
> **Purpose:** Give artists and developers a shared initial contract for preparing, naming, validating, and integrating game assets. Values not established by the project or current loader are marked **TBD**. This document does not approve an art style beyond the existing 2D Pixel Art direction.

## 1. Visual direction

- Game format: 2D.
- Art direction: Pixel Art.
- Camera direction: Top-Down.
- World setting: Fantasy.
- Visual consistency, readability, and correct transparent edges are important for sprites and UI assets.

Specific palette, pixel density, reference resolution, scaling policy, outline/shading rules, and final style guide remain TBD.

## 2. Asset categories

The following categories are expected to be needed. Their final contents and priorities must follow the GDD and stage designs.

| Category | Examples / intended use | Status |
|---|---|---|
| Character sprites | Player and other characters | Required category; exact roster TBD |
| Character animation sheets | Idle, walk, run and later approved actions | Current prototype supports named clips |
| Environment tiles | Ground and world surfaces | Required category; final tileset format TBD |
| Environment details | Vegetation, rocks, props, decorative overlays | Prototype world supports up to two tile overlays |
| Stage props | Cage, doors, furniture, interactive objects | Level-specific asset list TBD |
| UI graphics | Buttons, panels, joystick and indicators | Final visual specification TBD |
| Icons | Items, status, interactions and menus | Exact inventory TBD |
| Effects | Impact, magic, environmental and feedback effects | Effects catalogue TBD |
| Narrative artwork | Story/cutscene illustrations if selected | Presentation method TBD |
| Audio | Music, ambience, dialogue and sound effects | Formats and audio pipeline TBD |
| Fonts | UI and supported-language text | Font family, licensing, glyph coverage and fallback TBD |

## 3. Image and transparency requirements

Initial handoff guidance:
- Supply transparent sprites and UI elements as PNG unless the team approves another format.
- Preserve alpha transparency; do not flatten sprites onto a colored background.
- Avoid unintended semi-transparent fringes around pixel-art edges.
- Keep source files and exported runtime files distinguishable.
- Do not add labels, mock UI, or background elements to an isolated asset unless the brief explicitly requests them.
- Do not crop away animation pixels, shadows, or intentional padding without confirming the frame bounds and pivot.

PNG is a recommended interchange format for the initial asset handoff, not a final engine-wide format decision.

## 4. Sprite sheets and animation data

The observed animation loader supports a text configuration that describes named clips and rectangular frames in a source image.

### 4.1 Current parser constraints

These are implementation constraints observed in the current development branch, not necessarily permanent production limits:

- Maximum 16 clips per animation set.
- Maximum 96 frames per animation set.
- Clip name buffer: 32 bytes, including the terminating null character.
- Frame coordinates and dimensions are stored as unsigned 16-bit values.
- Frame duration must be greater than zero and fit in an unsigned 16-bit value.
- Frame rectangles must fit inside the decoded source image.
- Clip playback mode is either `loop` or `once`.

### 4.2 Current text format

The parser accepts a format equivalent to:

```text
# Example only — replace coordinates and names with approved artwork data.
clip idle 180 loop
frame 0 0 64 64
frame 64 0 64 64
end

clip walk_down 100 loop
frame 0 64 64 64
frame 64 64 64 64
end

clip run_down 70 loop
frame 0 128 64 64
frame 64 128 64 64
end
```

Each `frame` line contains `x y width height` in pixels. The duration value is the time per frame in milliseconds. The example is illustrative only; its dimensions, timing, and animation content are not approved production values.

### 4.3 Required handoff information

For every animated character, provide:
- Character/asset identifier.
- Source image filename and dimensions.
- Clip names and intended use.
- Ordered frame rectangles for each clip.
- Frame duration in milliseconds.
- Loop or one-shot behavior.
- Facing directions included in the sheet.
- Whether horizontal mirroring is intended and visually safe.
- Pivot/foot-point convention and any required offsets (currently TBD).
- Any frames that require special gameplay events or timing (TBD until mechanics are specified).

The current player mapping includes idle, walk, and run naming. Do not assume that this is the complete animation list for all characters.

## 5. World tile and overlay data

The observed prototype world data currently defines:
- A 30 × 30 grid.
- A tile-size constant of 128 world units.
- A maximum of two decorative overlays per tile.
- 13 available overlay identifiers in the current data definition.
- A deterministic seed-based variation/decorative placement function.

These values describe the current prototype data model only. They do not define the final world scale, final tileset dimensions in pixels, or final map authoring format.

The team must document the meaning of each tile/overlay ID and its corresponding image before adding or changing IDs. The mapping between IDs and actual art assets is TBD in this document.

## 6. Naming and directory conventions

A consistent naming convention is needed, but the final directory layout has not yet been approved.

Proposed naming principles for review:
- Use lowercase ASCII filenames and underscores.
- Use stable descriptive identifiers rather than temporary names.
- Include character or environment identity in filenames.
- Keep animation configuration names consistent with the clip names consumed by code.
- Avoid spaces and ambiguous version suffixes such as `final2_new`.
- Keep source artwork separate from runtime exports where the repository structure allows it.

Suggested examples (proposals only):
- `player_sprite_sheet.png`
- `player_animations.txt`
- `environment_grass_tile.png`
- `ui_virtual_joystick_base.png`

The final asset directory, case policy, atlas policy, and versioning method remain TBD.

## 7. Resolution, scaling, and filtering

- Pixel Art is the approved direction.
- The inspected sprite texture path currently uses Vulkan nearest-neighbor filtering.
- The final reference resolution, pixels-per-world-unit relationship, integer scaling policy, viewport adaptation, and UI scaling rules remain TBD.
- Artists should not independently resize exports to arbitrary screen resolutions. The project should establish a single scaling policy before bulk production.

## 8. File size and runtime constraints

The observed sprite-loading code currently rejects an encoded asset larger than 8 MiB and rejects decoded image dimensions above 4096 × 4096 in that path. These are code-level guards, not a target size budget for all assets.

The team should track:
- Source file size.
- Decoded memory footprint.
- Texture dimensions and format.
- Number of simultaneously resident textures.
- Upload time and loading behavior.

No final per-asset, per-atlas, or total memory budget has been approved.

## 9. Localization and typography

The game is expected to require multilingual support, but the final supported-language list and font strategy are not yet fixed in this specification.

Before selecting fonts, verify:
- Required glyph coverage for every approved language.
- Correct shaping and text direction for right-to-left scripts.
- Fallback behavior for missing glyphs.
- Readability at the target display scale.
- Font redistribution and embedding license.

Do not assume a Latin-only font is sufficient for the final game.

## 10. Asset review and acceptance checklist

An asset is ready for integration only after the relevant checks are completed:

- [ ] Matches the approved visual brief and game context.
- [ ] Correct file type and dimensions are recorded.
- [ ] Transparency is preserved where required.
- [ ] No unintended background, label, or UI element is baked into an isolated asset.
- [ ] Pixel edges and alpha fringe have been inspected at native scale.
- [ ] Animation frame rectangles fit the source image.
- [ ] Animation names match the configuration and runtime mapping.
- [ ] Loop/one-shot behavior and frame timing are documented.
- [ ] Pivot/origin and foot-point alignment are documented where relevant.
- [ ] File size and decoded memory impact are recorded.
- [ ] Licensing/ownership and source provenance are documented.
- [ ] Asset has been tested in the actual Android/Vulkan build.

An unchecked item is not automatically a rejection; it means the relevant review has not yet been recorded.

## 11. Open production decisions

| Topic | Status |
|---|---|
| Final reference resolution | TBD |
| Pixel density / world-unit mapping | TBD |
| Final tile pixel dimensions | TBD |
| Sprite pivot/origin convention | TBD |
| Final atlas and packing policy | TBD |
| Final asset directory layout | TBD |
| Naming and versioning policy | TBD |
| Approved image/audio formats | TBD |
| Audio pipeline and codecs | TBD |
| Font families and licenses | TBD |
| Final language list and glyph requirements | TBD |
| Asset size and memory budgets | TBD |
| Asset review owner and approval workflow | TBD |

## 12. Change control

Asset format or naming changes that affect runtime code must be coordinated with the engineering team. Art direction changes that affect the game's visual identity require project creator approval. Update this document when a production standard is approved; do not silently convert a proposal into a mandatory rule.
