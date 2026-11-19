# TORNIA-GULD — Game Design Document (GDD)

> **Document status:** Living draft — Version 0.1  
> **Project:** Tornia-guld  
> **Owner / final creative authority:** Project creator  
> **Purpose:** Central design reference for the game's player experience, narrative presentation, gameplay, systems, content, interface, and production decisions.  
>
> **Accuracy rule:** This document distinguishes confirmed decisions from open questions. Unknown details must remain marked as **TBD** until the project creator decides them. Do not infer story secrets, character facts, mechanics, or world rules.

---

## 1. Game at a Glance

| Field | Current information |
|---|---|
| Working title | Tornia-guld |
| Genre | TBD |
| Game format | 2D |
| Visual direction | Pixel Art |
| World / setting | Fantasy world with magical elements |
| Initial platform | Android |
| Initial target ABI | arm64-v8a |
| Primary development language | C |
| Graphics API | Vulkan |
| Engine | Custom engine |
| Camera / viewpoint | TBD |
| Player mode | TBD |
| Business model | TBD |
| Target audience / age rating | TBD |
| Supported languages | TBD |

### 1.1 High-level concept
Tornia-guld is a 2D Pixel Art game set in a fantasy world with magical elements. Its opening follows a child who is thrown into a river, rescued by a beggar, sold to a slave trader, and kept in captivity for ten years. The opening moves from narrative scenes into the first playable escape-and-survival sequence.

The broader premise, central player fantasy, long-term objective, and full-game structure have not yet been defined in this document.

### 1.2 Design pillars
The creator has identified the following core pillars:
- **Adventure:** exploration and discovery are central parts of the experience.
- **Holistic character progression:** the protagonist develops in both material and spiritual/personal dimensions.
- **Emotional experience:** events and relationships should carry emotional weight for the player.
- **Meaningful choices and consequences:** player decisions affect relationships with others and future opportunities.

The relative priority, detailed mechanics, and full-game implementation of these pillars remain to be defined.

---

## 2. Opening Flow and First-Time Player Experience

### 2.1 Current opening flow
The currently approved high-level flow is:

1. The player launches the game.
2. The main menu is shown.
3. Selecting **START GAME** leads directly into the story scenes.
4. There is no additional standalone world-introduction scene before the river story.
5. The narrative proceeds toward the first playable sequence in the cage.

The exact presentation method for the story scenes (for example, animated scenes, illustrated panels, or playable scenes) is **TBD**.

### 2.2 Main menu — implemented prototype
The current prototype contains:
- Game title: **TORNIA GULD**
- **START GAME** button
- **EXIT** button
- START GAME currently displays **COMING SOON** because the game scene/gameplay flow is not implemented yet.
- EXIT closes the application.

The final menu art, animation, sound, transitions, settings access, save/load access, and additional menu options are **TBD**.

### 2.3 Opening narrative sequence
The following order reflects the current story documentation:

1. **The river rumor:** A beggar searches near a river bridge on a rainy night. He sees a woman throw something into the river, follows, and discovers that it is a child. He rescues the child, then sells the child to a small slave trader because of poverty and need for money.
2. **Ten years in captivity:** The child grows up isolated in a cage/room, with little human contact and repeated abuse by a guard.
3. **The slave market night:** The protagonist hears that a large slave market and auction are being prepared and realizes they may be moved to a worse place.
4. **The centipede and ant:** The protagonist observes an ant attacking and apparently killing a centipede. The protagonist identifies emotionally with the small ant. The meaning, unusualness, and any possible magical significance of this event remain unresolved.
5. **The guard enters:** The guard who regularly beats the protagonist enters and makes a sexual threat. The protagonist retreats and says, “Don't come closer.” **No sexual assault occurs in the current documented events.**
6. **First playable sequence:** The player tries to avoid being caught in the cage for one minute. The guard then catches and throws the protagonist down. The protagonist accidentally grabs the guard's dagger and stabs the guard in the throat, killing the guard.
7. **Aftermath:** The protagonist hears other guards upstairs, searches the dead guard, finds a key, a dagger, and 10 copper coins, then closes the cage door and stands behind it.

**Important narrative constraints**
- The protagonist's response “Don't come closer” is motivated by fear, not a deliberate display of bravery.
- The guard's threat must not be rewritten as an assault; no assault occurs in the currently documented scene.
- Do not decide what the ant is, what its glance means, or whether it is magical without the creator's direction.
- Do not invent the protagonist's name, gender, exact age, family, origin, or powers.
- Do not resolve open mysteries or add story events without approval.

---

## 3. Story and Narrative Design

### 3.1 Narrative source of truth
- docs/STORY.md is the detailed narrative record.
- This GDD records the structure and the player-facing presentation of the narrative.
- If a narrative detail is not settled in STORY.md or explicitly approved by the creator, mark it **TBD**.

### 3.2 Narrative structure
- Opening / prologue: **TBD** (the river rumor is the first currently documented story event).
- Main chapters: **TBD**
- Endings: **TBD**
- Branching choices and consequences: **TBD**
- Dialogue system and dialogue presentation: **TBD**
- Narrator / point of view: **TBD**
- Cutscene skip and replay rules: **TBD**

### 3.3 Known characters
| Character | Confirmed information | Unresolved information |
|---|---|---|
| Protagonist | The child found in the river; held in captivity for ten years; central playable character | Name, gender, exact age, personality beyond known reactions, family, origin, abilities |
| Beggar | Finds and rescues the child, then sells the child due to poverty | Name, later role, motives beyond the recorded circumstances |
| John Clark | Small slave trader who buys the child; his business expands over ten years | Full personality, background, motives, later role and fate |
| Woman by the river | Throws the child into the river; wears clothing resembling a servant's, while riding in a luxurious carriage | Identity, motive, relationship to the child |
| Guard | Repeatedly beats the protagonist and later threatens them; is killed during the first playable sequence | Name, background, wider role |
| Other guards | Heard upstairs after the guard is killed; apparently drinking | Identities, number, behavior, response |

### 3.4 World and lore
- Fantasy setting with magical elements.
- World geography, nations, cities, cultures, history, factions, religions, magic rules, creatures, and social systems: **TBD**.
- The story's slavery system and the operation of slave markets require further worldbuilding: **TBD**.

---

## 4. Gameplay

### 4.1 Core gameplay loop
**TBD.** Define the recurring player activity across the full game, not only the first level.

### 4.2 Player abilities and actions
| Ability / action | Current status |
|---|---|
| Movement within the cage | Planned for Level 01; controlled by Virtual Joystick |
| Avoiding the guard | Planned for the one-minute opening challenge |
| Taking / using items | Item discovery is documented; interaction rules TBD |
| Combat | First guard kill is documented; wider combat system TBD |
| Stealth | TBD |
| Exploration | TBD |
| Dialogue / interaction | TBD |
| Magic / special abilities | TBD |

### 4.3 Controls
- Initial prototype input: touchscreen.
- Level 01 movement: Virtual Joystick.
- Other actions, contextual buttons, interaction controls, accessibility options, controller support, and remapping: **TBD**.

### 4.4 Difficulty and failure
- Level 01 contains a one-minute avoidance challenge.
- QTE rules are documented in docs/LEVEL_01.md.
- Failure states, checkpoints, retries, death rules, and difficulty settings: **TBD**.

### 4.6 Choices, consequences, and karma
- Player choices can have positive or negative consequences depending on the situation; the project does not impose a universal pre-labelled list of good and bad actions.
- Choices directly affect the protagonist's relationships with other characters.
- Choices can also affect future opportunities, including opportunities that may become available or be lost.
- **Karma is a deep, distinct system:** routine everyday actions do not increase it. Meaningful choices are what influence karma.
- Karma must not automatically be treated as the same thing as public reputation or other characters' opinions of the protagonist.
- The exact rules for evaluating choices, including the roles of intention, outcome, and the nature of the choice, remain **TBD** and require creator approval.
- **Communicating consequences:** use a combination of immediate clear feedback, gradual consequences revealed through later character behavior and events, and hidden consequences that may only become understandable later. The presentation method depends on the choice and its consequences.

### 4.5 Progression
Progression is intended to be holistic, with two broad dimensions:

**Material progression**
- The protagonist can develop practical capabilities and their material circumstances. Specific attributes, skills, equipment, resources, and unlocks remain **TBD**.

**Spiritual / personal progression**
- The protagonist's developing personality should reflect the player's choices and actions over time.
- The protagonist has an established identity, history, and some characteristics that remain fixed.
- Other aspects of personality, outlook, behavior, and relationships can change through the player's actions and decisions.
- This is not intended to be reduced to a simple good/evil meter. The exact traits, presentation, and mechanics remain **TBD**.

---

## 5. First Level — Level 01

See docs/LEVEL_01.md for the detailed level specification.

### 5.1 Current sequence
1. The protagonist is confined in a cage with the guard.
2. The player avoids being caught for one minute, moving within the cage.
3. After the timer ends, the guard catches and throws the protagonist down.
4. The protagonist accidentally takes the guard's dagger and stabs the guard in the throat; the guard dies.
5. A QTE sequence uses four options. An option turns red and must be pressed three times within five seconds. Exact sequencing and failure behavior remain unresolved.
6. The protagonist gets up and hears other guards upstairs.
7. The protagonist searches the dead guard and finds a key, a dagger, and 10 copper coins.
8. The protagonist closes the cage door and stands behind it.

### 5.2 Level completion
The documented endpoint is the protagonist closing the cage door and standing behind it. Whether this is a level-complete trigger, a transition to another scene, or a handoff to player control is **TBD**.

---

## 6. Player Character, Attributes, and Status

The current Level 01 HUD proposal includes:
- Health
- Awareness
- Energy

Their exact meanings, ranges, regeneration/depletion rules, UI representation, and gameplay effects are **TBD**.

Additional character attributes, needs, conditions, inventory capacity, equipment slots, skills, and status effects: **TBD**.

---

## 7. Items, Inventory, and Economy

### 7.1 Confirmed Level 01 items
| Item | Quantity | Context |
|---|---:|---|
| Key | 1 | Found on the dead guard |
| Dagger | 1 | Found on the dead guard; the protagonist used the guard's dagger to kill him |
| Copper coins | 10 | Found on the dead guard |

### 7.2 Systems to define
- Inventory layout and capacity — **TBD**
- Item categories and item properties — **TBD**
- Equipment and weapon handling — **TBD**
- Currency types and denominations — **TBD**
- Shops, trading, loot, crafting, and resource sinks — **TBD**
- Item persistence and loss rules — **TBD**

---

## 8. Environments, Levels, and World Navigation

- First known environment: the protagonist's cage/room.
- Other known story locations: river bridge, slums, slave-trading premises, and the large slave market.
- Level list, world map, navigation model, environment interaction, hazards, environmental storytelling, and level transitions: **TBD**.

---

## 9. User Interface and HUD

### 9.1 Existing prototype
The implemented main menu is described in Section 2.2.

### 9.2 Level 01 HUD proposal
- Health
- Awareness
- Energy
- One-minute timer during the avoidance sequence
- Four-option QTE interface (exact layout TBD)
- Item pickup feedback (TBD)

### 9.3 Full UI inventory — to be specified
- Main menu and pause menu
- HUD
- Inventory
- Character/status screen
- Quest/objective log
- Dialogue interface
- Map
- Settings
- Save/load interface
- Notifications and tutorials
- Accessibility and language selection

Do not assume every interface above belongs in the final game; confirm its need and behavior with the creator.

---

## 10. Audio and Music

- Music direction: **TBD**
- Ambient sound: **TBD**
- Character voice / dialogue voice: **TBD**
- Sound effects for movement, combat, UI, environment, and interactions: **TBD**
- Audio settings and accessibility: **TBD**

---

## 11. Art and Animation

### 11.1 Confirmed visual direction
- 2D Pixel Art.
- Fantasy world with magical elements.

### 11.2 To define
- Pixel resolution / internal render resolution
- Sprite scale and tile size
- Palette and lighting rules
- Character proportions and animation frame rate
- Environment art direction
- Effects, particles, and magic visuals
- UI art direction and typography
- Animation standards and asset naming conventions

All values are **TBD** until approved.

---

## 12. Technical and Platform Constraints

The technical implementation reference is docs/PROJECT_OVERVIEW.md.

Current project decisions:
- Language: C
- Graphics API: Vulkan
- Custom engine
- Android NativeActivity / Native App Glue
- Android NDK and Clang
- Initial ABI: arm64-v8a
- Initial minimum Android API: 24
- Initial target Android API: 34
- NDK version: 26.1.10909125
- Build Tools: 34.0.0
- Vulkan 1.0 baseline

The GDD describes the intended game experience; PROJECT_OVERVIEW.md remains the source of truth for build and architecture details.

---

## 13. Localization and Accessibility

- Initial development platform: Android.
- Supported game languages: **TBD**.
- Text expansion, font rendering, right-to-left layout, subtitle behavior, and localization workflow: **TBD**.
- Control accessibility, readable UI scaling, color/contrast options, and motion/audio options: **TBD**.

---

## 14. Save, Settings, and Player Data

- Save system and save frequency: **TBD**
- Checkpoints and recovery: **TBD**
- Multiple save slots: **TBD**
- Pause behavior: **TBD**
- Settings categories: **TBD**
- Data persistence and migration: **TBD**

---

## 15. Production Scope and Milestones

### Current implemented foundation
- Android application skeleton.
- Vulkan rendering foundation.
- Touch input foundation.
- Main menu prototype with START GAME and EXIT.
- Automated Android build workflow.

### Current content status
- Story opening documented.
- Level 01 design documented, with unresolved mechanics.
- Full game scope and chapter plan: **TBD**.

### Proposed documentation / production gates
These are workflow suggestions, not approved production commitments:
1. Complete the GDD through creator interview and review.
2. Resolve the opening scene presentation and transition into Level 01.
3. Finalize Level 01 mechanics and acceptance criteria.
4. Define reusable player, input, UI, scene, resource, and save systems.
5. Implement and test the first complete playable sequence.
6. Expand the game plan level by level.

---

## 16. Open Questions and Decision Log

This section is the active checklist for completing the GDD. Answers should be added here and then reflected in the relevant sections.

### A. Game identity
- [ ] What is the exact genre or genre combination?
- [ ] What is the intended core player experience?
- [ ] Is the game primarily story-driven, exploration-driven, action-driven, survival-driven, or a combination?
- [ ] Is the game single-player?

### B. Camera, movement, and interaction
- [ ] What camera perspective will the game use?
- [ ] Is movement free in 2D, platform-based, grid-based, or something else?
- [ ] How does the player interact with objects and characters?
- [ ] Which controls are always visible, and which are contextual?

### C. Opening presentation
- [ ] How should the river scene and subsequent opening events be presented: animated cutscenes, illustrated panels, in-engine scenes, playable sequences, or a combination?
- [ ] Should the player have any control before the cage sequence?
- [ ] How should dialogue, narration, and internal thoughts appear?
- [ ] What transitions connect the opening scenes?

### D. Full-game structure
- [ ] What is the protagonist's long-term objective?
- [ ] What is the broad structure after Level 01?
- [ ] Is the world divided into levels, regions, chapters, or an interconnected map?
- [ ] Are there branching paths or player choices?
- [ ] How far-reaching and persistent can the consequences of choices be?
- [x] How should the player learn about consequences? Use a context-sensitive combination of immediate feedback, gradual revelation, and hidden consequences.

### E. Character and systems
- [ ] Which character attributes and needs exist?
- [ ] What do Health, Awareness, and Energy each represent?
- [ ] Are there skills, experience, levels, or other progression systems?
- [ ] Which aspects of the protagonist's personality are fixed, and which can change?
- [ ] How are material and spiritual/personal progression represented to the player?
- [ ] How does the karma system evaluate meaningful choices (intention, outcome, nature of the choice, or a combination)?
- [ ] Are karma effects visible, hidden, or partly discoverable by the player?
- [ ] What are the combat, stealth, and survival rules?
- [ ] What inventory, equipment, and economy systems are required?

### F. Content and presentation
- [ ] What are the intended art and animation standards?
- [ ] What is the audio and music direction?
- [ ] Which languages are required for the first release?
- [ ] Which accessibility options are required?

### G. Product and release
- [ ] Is Android the only planned platform, or only the first platform?
- [ ] What is the intended business model?
- [ ] What age rating and content boundaries should guide production?
- [ ] What is the intended scope of the first public demo?

### Decision log
| ID | Decision | Status | Notes |
|---|---|---|---|
| D-001 | Use 2D Pixel Art presentation | Confirmed | Project direction |
| D-002 | Use a fantasy world with magical elements | Confirmed | Detailed lore remains open |
| D-003 | Start the story directly after START GAME, with no extra world-introduction scene beforehand | Confirmed | Exact scene presentation remains open |
| D-004 | First playable sequence is the cage escape-and-survival sequence in Level 01 | Confirmed in current story/level documents | Detailed interaction rules remain open |
| D-005 | Use C, Vulkan, a custom engine, and Android NativeActivity for the initial technical foundation | Confirmed | See PROJECT_OVERVIEW.md |
| D-006 | Core experience pillars are adventure, holistic progression, emotional impact, and meaningful choices with consequences | Confirmed | Relative weighting and implementation remain open |
| D-007 | Character progression has material and spiritual/personal dimensions | Confirmed | Detailed attributes and mechanics remain open |
| D-008 | The protagonist has some fixed identity traits and some personality traits that can change through player choices and actions | Confirmed | Exact fixed and mutable traits remain open |
| D-009 | Player choices affect relationships and future opportunities | Confirmed | Scope and persistence of consequences remain open |
| D-010 | Karma is a deep system influenced by meaningful choices, not routine everyday actions | Confirmed | Evaluation rules (intention, outcome, nature of choice) remain open |
| D-011 | Consequences are communicated through a context-sensitive combination of immediate, gradual, and hidden feedback | Confirmed | Which method applies depends on the choice and its consequences |
