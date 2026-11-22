# Player entity, movement and camera

## Implemented prototype

- The player is a world entity with a world-space foot position (`x`, `y`), velocity, normalized movement input, facing direction and motion state.
- The player is initialized near the center of the 30 x 30 world.
- Movement is driven by a virtual joystick and updated using elapsed frame time.
- Joystick input supports continuous 360-degree movement. The current sprite sheet has four directional walk/run clips, so diagonal input selects the dominant horizontal or vertical facing direction while retaining normalized diagonal movement speed.
- A high joystick deflection selects run; lower deflection selects walk. Releasing the stick selects idle.
- The current prototype limits the player's foot position to the world rectangle. It does not yet collide with vegetation, walls, props or enemies.

## Camera behavior

- The camera follows the player by default and uses a smoothed world-space target.
- The player is framed at approximately 50% of viewport width and 55% of viewport height (the player's position is the foot anchor).
- The camera continues to respect world bounds.
- Touch-drag outside the virtual joystick manually pans the camera. Manual pan temporarily disables automatic follow.
- Touching and moving the virtual joystick re-enables camera follow. This keeps manual camera inspection available while the movement controller is being developed.
- The manual-pan gesture is deliberately retained for this prototype. It can later be disabled or moved behind a setting when the controller scheme is finalized.

## Prototype tuning values

These are implementation defaults for testing, not final game-design commitments:

- Walk speed: 145 world units per second.
- Run speed: 225 world units per second.
- Run threshold: 82% of joystick travel.
- Character sprite height: 0.78 world tiles (approximately 100 world units at the current 128-unit tile size).
- Camera follow responsiveness: 8.0.

The character is rendered from its sprite-sheet source rectangles at the world-space player position. Its screen size follows camera zoom, and nearest-neighbor sampling preserves the pixel-art look.

## Next work

- Test the new controls and camera on an Android device.
- Add collision handling for solid world objects when their collision data is defined.
- Tune player speed, sprite scale, joystick placement and camera response from device feedback.
- Expand animation state selection for additional actions and future controller types.
