# Character movement and animation integration

## Reusable runtime model

Each character instance owns its own movement state and animation playback state:

- `Player`: world-space position, velocity, normalized input, movement speeds, facing and motion state.
- `AnimationController`: active clip, frame index, elapsed frame time, completion state and horizontal flip state.
- `AnimationSet`: immutable clip/frame metadata for a sprite sheet. Multiple characters may share the same set when they use the same sheet.

The controller does not own or decode image data. Rendering receives the current frame rectangle and flip flag, then samples the shared Vulkan texture. This keeps simulation, animation playback and rendering separate.

## State transition behavior

- Selecting the same clip on consecutive updates advances the existing playback; it does not restart the clip.
- Selecting a different valid clip starts that clip at frame zero.
- An unknown clip is rejected and does not overwrite the currently active clip.
- Horizontal mirroring is stored per character instance, not globally.
- Each character needs a separate `AnimationController`, even when sharing one `AnimationSet`.

## Adding another character

For a character using the existing sprite sheet:
1. Initialize a new `Player` with its own world position and movement settings.
2. Initialize a new `AnimationController`.
3. Update that character's controller with its selected clip, flip state and frame delta.
4. Render the controller's current frame at that character's world position.

For a character using a different sprite sheet, load a separate `AnimationSet` and texture resource, then use the same movement and controller APIs.

## Current scope and limitations

- The runtime currently instantiates one player in the world scene. The data model and controller API support independent per-character playback, but a multi-character entity collection, NPC AI, collision and entity lifecycle are not implemented by this stabilization.
- Current player clip mapping remains in `player_animation_name()`; it selects idle, walk and run states from movement direction.
- This document describes the implemented reusable playback foundation, not a claim that a full entity-component system or NPC framework exists.
