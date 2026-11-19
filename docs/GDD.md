# TORNIA-GULD — Game Design Document (GDD)

> **Document status:** Living draft — Version 0.1  
> **Project:** Tornia-guld  
> **Owner / final creative authority:** Project creator  
> **Purpose:** Central design reference for the game's player experience, narrative presentation, gameplay, systems, content, interface, and production decisions.  
>
> **Accuracy rule:** This document distinguishes confirmed decisions from open questions. Unknown details must remain marked as **TBD** until the project creator decides them. Do not infer story secrets, character facts, mechanics, or world rules.

---


## World Structure

- The game world uses a **continuous outdoor world**: the player can travel through connected outdoor areas such as streets, cities, wilderness, and routes without treating each area as a separate level.
- Certain locations, such as houses, shops, important buildings, dungeons, and story-specific places, may use **independent interior spaces** entered from the outdoor world.
- The intended experience is a continuous world with separate interior spaces, rather than one single environment containing every interior at all times.
- The exact streaming, loading, transition, and world-boundary implementation remains **TBD** and will be determined during technical level/world design.

## World Scale, Freedom, Time, and Travel

- The intended world is **open and geographically continuous**. The player should not face arbitrary region locks as the default world design; natural, physical, narrative, or capability-based obstacles may still exist where appropriate. Exact access rules remain **TBD**.
- The intended geography should have meaningful real-world-like distances and physical relationships. The creator's example is that two villages may be 10 km apart; the world should not treat such distances as arbitrary short transitions.
- The proposed global time scale is **30 minutes of real time = 1 in-game day (24 in-game hours)**. The world clock advances while the game is being played and pauses when the game is closed. Exact clock behavior while actively playing, pause-menu behavior, and simulation scheduling remain **TBD**.
- Travel should use a **hybrid approach**: the player can experience journeys and encounters along the route, while a faster/abstracted travel option may be available where appropriate. Exact eligibility, interface, and implementation remain **TBD**.
- Travel duration should be grounded in physical factors rather than a fixed arbitrary rule. Relevant factors may include distance, terrain, route quality, movement speed, character condition, equipment, weather, and other travel circumstances. The exact simulation model remains **TBD**.
- Journeys should provide opportunities for meaningful world encounters and discoveries. Specific encounter types, frequency, and procedural/story-authored balance remain **TBD**.
- A 10 km journey is not inherently a four-day walk under ordinary conditions; duration should emerge from the travel model. For example, at 4 km/h, 10 km takes about 2.5 in-game hours, before stops or complications.


## World Simulation Principles

- **World time pauses when the game is closed.** No world simulation, travel progress, encounter resolution, or other world event advances while the player is outside the game. On return, the world resumes from its saved in-game time and state.
- The world is intended to follow **consistent, understandable internal laws**. The creator describes this design philosophy as viewing the world from the perspective of a wise, just creator: its systems should be lawful and fair, not arbitrary.
- The world is not necessarily safe or balanced around the player's current strength. Powerful creatures, dangerous regions, and large-scale conflicts may exist even when the protagonist is not strong enough to overcome them.
- A fair world does not mean every encounter must be winnable. Outcomes should arise from coherent rules and relevant circumstances rather than arbitrary punishment. The exact warning cues, encounter logic, combat rules, and role of chance remain **TBD**.

## Camera and Viewpoint

- **Perspective:** 2D Top-Down (view from above).
- **Camera behavior:** Hybrid. During ordinary exploration, the camera follows the player character. In selected stages or situations, it may use a different behavior to support the intended presentation and gameplay.
- **Movement:** Free 360-degree movement in the 2D world is the confirmed direction; movement is not restricted to four or eight discrete directions.
- **Movement and attack direction:** The player may be able to move in one direction while attacking in another. This is a confirmed design principle, but it must be governed by specific combat mechanics rather than being universally available without restrictions. Weapon-specific rules, timing, accuracy, stamina/cost, animation, and other conditions remain **TBD**.
- The exact rules for when and how the camera changes behavior, including any fixed shots, framing, transitions, rotation, and zoom, remain **TBD** and must be defined in the relevant stage designs.

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
| Camera / viewpoint | 2D Top-Down (view from above) |
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
| Magic / special abilities | A broad, learnable field; a character may learn multiple forms of magic (for example, stone magic and water magic). All magic consumes Mana. Environmental presence of a corresponding element can broaden that element's magical power and unlock certain conditions or possibilities. Some spells may additionally require Stamina and/or Intelligence. Innate aptitude influences learning speed, mastery ceiling, and magical power/efficiency. Magic combinations, including combinations of more than two forms, are allowed only within combination rules and outcomes defined by the developers. Exact rules remain TBD |


### 4.2.1 Magic system — confirmed design direction

- Magic is a broad field of knowledge and practice, not a set of mutually exclusive character classes. A character may learn multiple forms, such as stone magic and water magic.
- Magical aptitude is relevant to learning speed, attainable mastery, and the power or efficiency of magical execution. Low aptitude does not automatically prevent learning or progress.
- **Mana is consumed whenever magic is used.** Some individual spells or techniques may also require other attributes/resources, including Stamina and/or Intelligence. Exact costs and thresholds remain **TBD**.
- The presence of a relevant element in the environment can expand the effective scope or power of that element's magic and may enable certain additional conditions or applications. Exact environmental rules remain **TBD**.
- **Magic combinations are permitted**, including combinations of more than two forms. However, combinations and their possible outcomes must remain within the combination rules and outcomes explicitly designed/defined by the developers. Player experimentation takes place within this authored system; it does not create arbitrary effects outside the world's rules.
- The exact catalogue of magical forms, spells, combination rules, environmental interactions, costs, and progression mechanics remains **TBD**.

### 4.2.2 Rise system — initial distinction only

- The creator has identified a separate progression/power system referred to as **Rise** (English spelling/name is provisional and remains **TBD**).
- Rise is distinct from magic and is strongly associated with physical capabilities and combat-related development, including stamina, strength, speed, and physical skills.
- A character may develop both magic and Rise; the systems are not mutually exclusive.
- Rise is relevant as a path for characters such as warriors and crusaders, and for characters whose Mana is weak.
- Rise is organized around distinct physical/combat skills, rather than being only a set of passive attribute bonuses. Each skill can have its own levels and unlocks.
- Example supplied by the creator: **Swordsmanship Level 1** enables basic sword attacks and defense; **Level 2** can improve attack power and speed; **Level 3** can unlock a named technique such as **Moon Sword** (شمشیر ماه). This is an illustrative progression example, not yet a complete skill specification.
- Skill levels may therefore provide both improvements to existing actions and access to new techniques directly associated with that skill.
- **Skill proficiency increases through actual use and practice.** Repeatedly using a skill develops the character's proficiency in that skill.
- **Awareness and discovery of innate aptitudes use a mixed approach.** Some aptitudes may be known from the beginning; others may become apparent later. Some may be hinted at, while others may have no explicit indication before discovery. Exact discovery methods remain **TBD**.
- **Inheritance of innate aptitudes is variable.** A child may inherit an aptitude from a parent, but inheritance is not guaranteed; the exact rules and probabilities remain **TBD**.
- **Innate aptitudes may vary in number and combination between characters.** A character may possess multiple aptitudes, while another may have a different or more limited combination. No universal fixed aptitude count has been established; distribution and limits remain **TBD**.
- **Innate aptitude also applies to Rise skills.** Having a natural aptitude for a particular physical/combat skill (for example, swordsmanship) affects learning and development in the same broad ways as magical aptitude: learning speed, attainable mastery, and the power or effectiveness of performance. The exact formulas and limits remain **TBD**. This aptitude is skill-specific; having an aptitude for swordsmanship does not automatically establish aptitude for every other Rise skill.
- **Training with a teacher is complementary to personal practice.** A teacher can introduce and teach different techniques; the character must then practice those techniques to develop proficiency in them.
- Learning a technique and advancing that technique to its next stage are distinct. Progression to a higher technique stage requires specific conditions, not merely repeated use.
- Creator example: the technique **Moon Sword** (شمشیر ماه) may advance to **Red Moon** (ماه قرمز). This advancement requires killing many humans and satisfying additional conditions. The number of kills and the other conditions have not yet been specified and remain **TBD**. This is an example of a condition-gated technique evolution, not a universal requirement for every technique.
- The complete skill list, level requirements, exact practice/proficiency model, teacher-training rules, technique catalogue, advancement conditions, numerical effects, and the relationship between Rise skills and general attributes remain **TBD**.

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

All characters have the following six general attributes: **Strength, Agility, Intelligence, Stamina, Mana, and Health**. These are the confirmed current attribute categories. Their exact definitions, ranges, individual values, and mechanical effects remain **TBD**.\n\n- **Innate aptitude can affect the starting value of some attributes.** For example, if an ordinary person's Strength is 10, a character with a relevant innate aptitude may begin with Strength 15 without having trained. The exact attributes affected, aptitude rules, and numerical differences remain **TBD**.\n- **Attributes can increase through training and practice.** Repeated or sustained training is a direct cause of attribute growth; innate aptitude is not a prerequisite for improvement. The training method appropriate to each attribute, the amount of growth, progression rates, limits, and attribute-specific rules will be designed later and remain **TBD**. Any examples of exercises are illustrative only, not confirmed mechanics.

Additional character needs, conditions, inventory capacity, equipment slots, skills, and status effects: **TBD**.

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
- [x] What camera perspective will the game use? 2D Top-Down.
- [x] How should the camera behave? Hybrid: follow the player during ordinary exploration, with different behavior allowed in selected stages or situations.
- [x] Is movement free in 2D, platform-based, grid-based, or something else? Free 360-degree movement in the 2D world.
- [x] Can movement direction and attack direction differ? Yes, subject to specific combat mechanics; exact restrictions and weapon rules remain TBD.
- [x] How should enemy targeting work? Hybrid approach: manual enemy selection, directional/free aiming, and lock-on may be used according to weapon and combat situation; exact rules remain TBD.
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
- [x] Should world time continue while the game is closed? No; the world simulation pauses outside active play and resumes from the saved state.
- [x] What is the world's governing design philosophy? Consistent internal laws and fairness without arbitrary protection or power-scaling around the player; precise mechanics remain open.
- [x] How should the player learn about consequences? Use a context-sensitive combination of immediate feedback, gradual revelation, and hidden consequences.

### E. Character and systems
- [x] Which broad resource types are intended? Separate Stamina, Energy, and Mana (MP) resources; their exact rules and uses remain TBD.
- [x] What enables a character to use magic? A special magical aptitude that may be innate or may awaken later in life; causes, conditions, prevalence, and development remain TBD.
- [x] What broad paths can develop magical ability? A combination of learning from teachers, personal practice/experience, and discovering ancient books, artifacts, or knowledge; exact mechanics remain TBD.
- [x] Is magic restricted to one innate element/type, or can characters learn multiple forms? Magic is a broad, learnable field; a character may learn multiple forms. Innate aptitude influences learning speed, mastery ceiling, and magical power/efficiency.
- [x] What resources does magic consume? All magic consumes Mana; some spells may additionally require Stamina and/or Intelligence. Exact costs and thresholds remain TBD.
- [x] How does the environment affect elemental magic? The presence of a relevant element can broaden that element's magical power and unlock certain conditions or applications; exact rules remain TBD.
- [x] Can players combine multiple forms of magic? Yes, including more than two forms, but only within combination rules and outcomes defined by the developers; exact catalogue and mechanics remain TBD.
- [x] Is there a progression system separate from magic for physical/combat development? Yes, the creator refers to it as Rise (provisional English spelling/name). It consists of physical/combat skills with their own levels and skill-linked unlocks. Example: Swordsmanship Level 1 enables basic attacks and defense; Level 2 can improve attack power and speed; Level 3 can unlock a specific technique such as Moon Sword. Characters may develop both Rise and magic. The complete system remains TBD.
- [x] Which general character attributes exist? Six categories: Strength, Agility, Intelligence, Stamina, Mana, and Health.\n- [x] How do innate aptitude and training affect attributes? Innate aptitude may raise the starting value of some attributes; attributes can also grow through training and practice, even without a relevant aptitude. Exact rules remain TBD.\n- [ ] Which additional character needs and status conditions exist?
- [ ] What do Health, Awareness, and Energy each represent?
- [ ] Are there skills, experience, levels, or other progression systems?
- [ ] Which aspects of the protagonist's personality are fixed, and which can change?
- [ ] How are material and spiritual/personal progression represented to the player?
- [ ] How does the karma system evaluate meaningful choices (intention, outcome, nature of the choice, or a combination)?
- [ ] Are karma effects visible, hidden, or partly discoverable by the player?
- [x] What is the broad combat structure? Hybrid real-time action: combat normally runs continuously, while selected abilities or mechanics may alter its flow or timing; exact effects remain TBD.
- [x] Which broad defensive methods should be supported? A combined system of blocking, dodging, and parrying; exact mechanics and requirements remain TBD.
- [ ] What are the detailed combat, stealth, and survival rules?
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
| D-012 | The world is continuous outdoors, with independent interior spaces for selected locations | Confirmed | Exact streaming and transition implementation remains open |
| D-013 | The intended world is open and geographically continuous, with no arbitrary region locks as the default | Confirmed direction | Natural, physical, narrative, or capability-based obstacles and exact access rules remain open |
| D-014 | World time is planned at 30 real-time minutes per in-game day; travel uses a hybrid experiential/fast approach | Confirmed direction | Travel physics, fast-travel rules, active-play clock behavior, and encounter scheduling remain open |
| D-015 | World simulation pauses completely when the game is closed | Confirmed | No offline travel progress or world events; resume from saved world state |
| D-016 | The world follows consistent, fair internal laws and is not artificially balanced to the player's strength | Confirmed design philosophy | Exact creature ecology, danger cues, encounter logic, combat rules, and chance remain open |
| D-017 | Use a 2D Top-Down camera perspective with hybrid camera behavior | Confirmed | Camera follows the player during ordinary exploration; selected stages or situations may use different camera behavior. Exact triggers and camera techniques remain TBD. |
| D-018 | Use free 360-degree movement and allow movement direction to differ from attack direction under specific combat mechanics | Confirmed design direction | The player may move one way while attacking another, but this is not unrestricted or universal. Weapon-specific rules and exact constraints remain TBD. |
| D-019 | Use a hybrid enemy-targeting approach | Confirmed design direction | Manual enemy selection, directional/free aiming, and lock-on targeting may be used depending on weapon and combat situation. Exact rules, switching behavior, input mapping, and compatibility remain TBD. |
| D-020 | Use hybrid real-time action combat | Confirmed design direction | Combat normally unfolds continuously in real time; selected abilities or mechanics may alter combat flow or timing. Whether they pause, slow, or otherwise modify time, and exact rules, remain TBD. |
| D-021 | Support blocking, dodging, and parrying as complementary defensive methods | Confirmed design direction | Exact mechanics, timing windows, effectiveness, risks, costs, and equipment/skill requirements remain TBD. |
| D-022 | Treat Stamina, Energy, and Mana (MP) as distinct resource concepts | Confirmed design direction | Stamina supports immediate physical actions; Mana powers magical abilities; Energy is a separate resource whose exact role remains TBD. Costs, recovery, limits, and interactions remain TBD. |
| D-023 | Magical ability requires a special magical aptitude/talent | Confirmed design direction | The aptitude may be innate from birth or awaken/become apparent later in life. Causes, conditions, prevalence, and development remain TBD; awakening does not necessarily grant immediate spellcasting ability. |
| D-024 | Magical progression can use multiple complementary paths | Confirmed design direction | Growth may come through teachers/instruction, personal practice and experience, and discovery of ancient books, artifacts, or knowledge. Relative contribution and exact mechanics remain TBD. |
| D-025 | Magic is a broad field that can be learned in multiple forms by the same character | Confirmed design direction | A character may learn more than one form of magic (for example, stone magic and water magic). Innate aptitude influences the degree or rate of mastery rather than automatically limiting the character to one form. Exact aptitude, learning, specialization, and mastery mechanics remain TBD. |
| D-026 | Innate magical aptitude can affect learning speed, mastery ceiling, and magical power/efficiency | Confirmed design direction | Innate aptitude may influence how quickly a character learns magic, the level of mastery they can reach, and the power or efficiency of their magic. Low aptitude does not prohibit learning or improvement. Exact formulas, limits, and balancing remain TBD. |
| D-027 | All magic consumes Mana; some spells may additionally require Stamina and/or Intelligence | Confirmed design direction | Exact costs and thresholds remain TBD. |
| D-028 | The presence of a relevant element in the environment can expand the scope or power of that element's magic and enable certain applications | Confirmed design direction | Exact environmental rules remain TBD. |
| D-029 | Magic forms may be combined, including combinations of more than two forms, within developer-defined rules and outcomes | Confirmed design direction | Exact combinations and effects remain TBD. |
| D-030 | Rise is a progression system distinct from magic, focused on physical/combat skills; a character may develop both systems | Confirmed design direction | Rise's English name is provisional. |
| D-031 | Rise skills improve through practical use and practice; teachers can teach techniques, while advancing a technique requires specific conditions | Confirmed design direction | Example: Moon Sword may advance to Red Moon after killing many humans and meeting additional conditions. Exact thresholds and other conditions remain TBD. |\n| D-032 | Technique advancement conditions may be a combination of different condition types | Confirmed design direction | The creator and development team will define and author the specific conditions later. Do not assume a fixed universal condition list; exact requirements remain TBD until authored. |
| D-033 | Innate aptitude can influence the development of individual Rise skills in the same broad ways as magical aptitude | Confirmed design direction | A character with an aptitude for swordsmanship can learn and develop it more readily, with aptitude affecting learning speed, attainable mastery, and performance power/effectiveness. Exact formulas and limits remain TBD. |
| D-034 | Characters may have varying combinations and numbers of innate aptitudes | Confirmed design direction | The number and combination of aptitudes are not fixed or uniform across all characters; an individual may have multiple aptitudes, while another may have a different or more limited combination. Exact distribution and limits remain TBD. |
| D-035 | Inheritance of innate aptitudes is variable, not guaranteed | Confirmed design direction | A child may or may not inherit an aptitude from their parents. Inheritance is not a fixed or guaranteed rule; exact inheritance probabilities and rules remain TBD. |
| D-036 | Awareness and discovery of innate aptitudes use a mixed approach | Confirmed design direction | Some aptitudes may be known from the beginning, while others may become apparent later. Some may have clues or indications; others may remain undiscovered without any explicit hint. Exact discovery methods remain TBD. |
| D-037 | All characters share six general attribute categories | Confirmed design direction | The six categories are Strength, Agility, Intelligence, Stamina, Mana, and Health. Exact definitions, ranges, values, progression rules, and mechanical effects remain TBD. |\n| D-038 | Some general attributes may receive a higher starting value from innate aptitude, and attributes can grow through training | Confirmed design direction | Example: an ordinary person's Strength may be 10, while a character with a relevant innate aptitude may start at 15 without training. Attributes can increase through training and practice regardless of whether the character has that aptitude. Which attributes are affected, exact values, training methods, rates, and limits remain TBD. |
| D-039 | Attribute-specific training methods and progression details will be defined later | Confirmed design direction | The principle that training can increase attributes is confirmed, but the appropriate training for each attribute, growth amounts, rates, limits, and detailed rules are intentionally deferred. Illustrative exercise examples are not approved mechanics. |
| D-040 | Attribute growth ceilings and age-related changes vary by character | Confirmed design direction | A character's potential ceiling depends on factors including innate aptitude, age/lifespan, and amount of training. Attributes may decline with age; for example, Strength generally begins to decrease after age 70, but exceptions exist. Exact formulas, age curves, attribute-specific effects, exception criteria, and the influence of lifespan remain TBD. |
| D-041 | Innate aptitude, age/lifespan, and training can directly affect all six general attributes | Confirmed design direction | This applies to Strength, Agility, Intelligence, Stamina, Mana, and Health—not only physical attributes. The specific way and degree each factor affects each attribute, including age-related increases or declines, remain TBD and must be defined individually. |
| D-042 | General attributes may also change over time without deliberate training | Confirmed design direction | Attributes may naturally increase or otherwise change as a person ages and gains life experience, even without targeted training. This is variable: some individuals may show little or no change. The nature and degree of change depend on the individual and their experience; exact attribute-specific patterns and mechanics remain TBD. |
| D-043 | Concrete life events and their underlying causes can directly affect character attributes | Confirmed design direction | Attribute changes arise from what actually happens to the character and the circumstances or causes behind an event, not simply from whether the character is labelled as having won or lost. Example: in a war, a character may suffer severe injuries whether their side wins or loses; those injuries can affect attributes. Exact event-to-attribute effects, severity, duration, and recovery rules remain TBD. |
| D-044 | Psychological trauma may affect character attributes depending on its severity and impact | Confirmed design direction | Experiences such as intense fear, witnessing the death of loved ones, or captivity may affect a character, depending on the psychological impact of the experience. Such effects are possible, not automatic; the nature, severity, duration, affected attributes, and recovery rules remain TBD. |
| D-045 | Difficult experiences may lead to growth in some attributes as well as harm | Confirmed design direction | A difficult or traumatic experience may, depending on the individual and the experience, contribute to the development of certain attributes or capabilities as well as causing harm. Growth is possible, not guaranteed; the attributes affected, causal conditions, magnitude, and persistence remain TBD. |
| D-046 | Characters have a separate category of personality traits in addition to the six general attributes | Confirmed design direction | Personality traits are a distinct category from the six general attributes. Examples may include willpower, courage, fear, and confidence, but the final list, definitions, representation, and rules for change or interaction remain TBD for later design. Personality traits have degrees/intensity levels rather than being only binary present-or-absent traits. Their intensity is represented on a continuous numeric scale from 0 to 100, allowing gradual changes rather than only fixed categories. The exact interpretation of values, trait-specific thresholds, and calibration remain TBD. Personality traits may change through life experiences, but such change is possible rather than guaranteed; its causes and mechanics remain TBD. Personality traits may also influence a character's decisions and behavior in different situations; the specific effects and mechanics remain TBD. Characters may have different personality traits, so two characters with identical general attributes may still behave and react differently; the generation and distribution rules remain TBD. Personality-trait combinations are generally open rather than restricted to a predefined set, but they must follow coherent rules that will be defined later. Traits that appear contradictory may be possible only where those later rules allow them; no specific combination is confirmed or ruled out at this stage. A character may also possess multiple strong personality traits simultaneously; their compatibility and interactions remain TBD. Personality traits may also influence a character's relationships and social interactions with others; the specific effects and mechanics remain TBD. Characters may deliberately work to develop or change their own personality traits through intentional efforts such as practice, learning, or targeted life experiences. Such development is possible but its methods, limits, conditions, and mechanics remain TBD. Personality traits may also change unintentionally as a result of life circumstances and experiences, without the character deliberately seeking that change. Such changes are possible rather than guaranteed; their causes, direction, intensity, duration, and mechanics remain TBD. The same experience may affect different characters in different ways depending on their individual personality traits and circumstances; divergent outcomes are possible but not guaranteed, and the relevant factors and mechanics remain TBD. |
| D-047 | Personality development may result from a combination of inheritance and life experience | Confirmed design direction; detailed rules deferred | The creator's current direction is a combination of inherited influences and development through life. The relative contribution of inheritance and environment/experience, inheritance patterns, and detailed formation rules are intentionally deferred for a dedicated design discussion and remain TBD. |
| D-048 | Personality traits use an opposing-axis model where appropriate | Confirmed design direction | Some personality traits may be defined as opposing ends of a shared dimension (for example, courage and fear), rather than as wholly independent values. This does not mean every trait must have an opposite or that one trait automatically eliminates the other. The exact trait pairs, scales, interaction rules, and exceptions remain TBD. |
| D-049 | Personality-trait intensity uses a continuous 0–100 numeric scale | Confirmed | The scale allows gradual changes in trait intensity. The interpretation of values, trait-specific thresholds, and calibration remain TBD. |
| D-050 | Opposing personality traits are represented as independent values | Confirmed design direction | Traits that may be considered opposites (for example, courage and fear) each have their own independent 0–100 intensity value. A character may therefore have high or low intensity in both traits at the same time; one does not automatically reduce or eliminate the other. The exact trait pairs, definitions, interactions, and behavioral effects remain TBD. |

| D-051 | Personality-trait changes may be temporary or lasting depending on the experience and its impact | Confirmed design direction | A change in a personality trait may be temporary or lasting. Its persistence depends on the type and intensity of the experience and the character's circumstances. Changes do not automatically become permanent, nor do they automatically return to their previous value over time. The exact duration, recovery, and persistence rules remain TBD. |

| D-052 | Personality traits may change gradually over time even without new triggering experiences | Confirmed design direction | Personality-trait intensity may change naturally and gradually as time passes, even when no new specific event or experience triggers the change. This is not universal: individual traits and characters may differ. Some traits may be deeply rooted (for example, deep-rooted fear) and may persist over time rather than naturally fading. The factors, rates, trait-specific exceptions, and detailed persistence rules remain TBD. |

| D-053 | Characters may intentionally attempt to change their own personality traits, but success depends on experiences and circumstances | Confirmed design direction | A character may consciously try to develop or change a personality trait (for example, trying to become more courageous). The attempt is possible, but success and the degree of change depend on the character's experiences and circumstances; deliberate effort does not guarantee the intended result. The relevant conditions, methods, limits, and mechanics remain TBD. |

| D-054 | Other characters may influence an individual's personality, but influence is not guaranteed | Confirmed design direction | Relationships and prolonged interaction with other characters may influence a character's personality traits (for example, through friendship or living alongside someone with a particular disposition). Such influence may occur or may not occur; exposure to another person's personality does not guarantee change. The likelihood, direction, magnitude, and conditions of influence remain TBD. |

| D-055 | A significant life event may change multiple personality traits at the same time | Confirmed design direction | A significant life event (for example, betrayal, love, failure, or the loss of a loved one) may affect multiple personality traits simultaneously. The affected traits, direction and magnitude of change, and whether the effects are temporary or lasting depend on the event and the individual character; no universal outcome is assumed. Exact causal rules remain TBD. |

| D-056 | Personality traits may influence a character's behavior in specific situations | Confirmed design direction | A character's personality traits may influence their behavior and responses in relevant situations (for example, a character with strong fear may be more likely to avoid danger or flee). Such influence is possible rather than guaranteed; it does not dictate a fixed response in every situation. The specific behavioral effects, probabilities, contextual factors, and interaction rules remain TBD. |

| D-057 | Personality traits may influence decision-making, allowing different characters to make different choices in the same situation | Confirmed design direction | Personality traits may influence how a character evaluates a situation and which decision they choose. Therefore, two characters facing the same situation may make different decisions because of differences in their personalities. This influence is possible rather than guaranteed; personality does not mechanically dictate a single outcome. The relevant traits, contextual factors, decision rules, and probabilities remain TBD. |

| D-058 | A character may sometimes act contrary to their personality traits | Confirmed design direction | A character may act contrary to their usual personality tendencies in a particular situation (for example, a highly fearful character may risk themselves to save a friend). This is possible but not guaranteed. Personality traits influence behavior without absolutely determining it; the situation, relationships, experiences, and other circumstances may contribute to an exception. The conditions, likelihood, and mechanics remain TBD. |

| D-059 | Characters may experience conflicting emotions or simultaneous feelings toward the same person or situation | Confirmed design direction | A character may experience apparently conflicting emotions or feelings at the same time (for example, fear and affection toward the same person). Such combinations are not restricted to a single personality trait or explanation; they may arise from different traits, relationships, experiences, desires, or circumstances. Their occurrence and intensity are not guaranteed, and the specific emotional categories, causes, interactions, and mechanics remain TBD. |

| D-060 | Characters may conceal their true emotions and display a different outward emotional state | Confirmed design direction | A character may hide or suppress what they genuinely feel and present a different emotion or outward behavior (for example, appearing happy while internally feeling sad or angry). The outward display does not necessarily reveal the character's actual emotional state. The ability and likelihood of concealment, its causes, duration, detection by others, and detailed mechanics remain TBD. |

| D-061 | Characters may infer other characters' emotional states from observable cues, with imperfect accuracy | Confirmed design direction | Characters may attempt to recognize another character's actual emotional state by interpreting observable cues such as behavior, tone of voice, facial expression, and other contextual signals. This ability is not guaranteed to succeed and is not perfectly accurate; a character may misinterpret or fail to notice concealed emotions. The relevant cues, individual differences, accuracy, and detailed mechanics remain TBD. |

| D-062 | Life experiences may improve or weaken a character's ability to interpret other people's emotions | Confirmed design direction | A character's experiences may affect how accurately or sensitively they interpret emotional cues in others. For example, repeated deception may make a character more alert to suspicious behavior, while other experiences may impair trust or interpretation. Such changes are possible rather than guaranteed and may vary by character and circumstance. The relevant experiences, direction, magnitude, persistence, and mechanics remain TBD. |


| D-063 | Character simulation should support interconnected, evolving human-like psychology while remaining computationally practical | Confirmed design principle | A character's personality, emotions, memories, experiences, relationships, and decisions may influence one another and contribute to an individual, evolving character over time. This is an aspirational design direction, not a requirement to simulate every psychological detail or every possible interaction. The system must be scoped and optimized so that its computational and memory costs remain practical for the target hardware and do not make the game unplayable. The appropriate level of detail, simulation methods, update frequency, and performance budget remain TBD. |

| D-064 | Character memories may continue to influence them long after the remembered event | Confirmed design direction | Memories are not merely a chronological record of past events. A remembered experience may continue to affect a character's emotions, relationships, judgments, trust, and decisions years later, including when the character is not consciously recalling the event. The strength, duration, accessibility, and specific effects of memories may vary by character and circumstance. Detailed memory formation, recall, forgetting, and computational implementation remain TBD. |

| D-065 | Character memories may fade or lose detail over time | Confirmed design direction | Characters may gradually forget experiences or lose some details of remembered events over time. Forgetting is not necessarily complete: some memories or important details may persist, while others become less accessible or less precise. This can also support computational efficiency by avoiding the need to retain and process every memory at full detail indefinitely. The criteria for memory importance, fading, retention, summarization, and implementation remain TBD. |

| D-066 | Environmental and social cues may reactivate faded memories | Confirmed design direction | A memory that has faded or become less accessible may become vivid or accessible again when a relevant cue is encountered, such as returning to a place, hearing a sound, seeing an object, or meeting a person associated with the event. Reactivation is possible rather than guaranteed and may vary by character, memory, and context. The cues, likelihood, degree of recall, emotional effects, and implementation remain TBD. |

| D-067 | Character recollections may differ from the objective events | Confirmed design direction | A character's memory of an event may be incomplete, subjective, or distorted rather than a perfectly accurate recording. Different characters who experienced the same event may remember or interpret it differently, and an individual's recollection may change over time. The causes, limits, reliability, and gameplay consequences of memory distortion remain TBD. |

| D-068 | Characters may form or hold false recollections and believe them to be true | Confirmed design direction | A character may have an inaccurate or distorted recollection yet sincerely regard it as factual and make judgments or decisions based on that belief. This may affect relationships, trust, interpretations, and later behavior. The causes, likelihood, persistence, correction, and gameplay consequences remain TBD. |

| D-069 | Responses to false memories may vary: characters may correct them or retain them | Confirmed design direction | A character may discover that a believed recollection is false or distorted, but this is not guaranteed. Another character may retain the false belief indefinitely, potentially for their entire life. Outcomes are individual and context-dependent; the factors, likelihood, timing, and mechanisms of correction or persistence remain TBD. |

| D-070 | Characters may form opinions and emotional responses based on other characters' accounts of events they did not experience | Confirmed design direction | A character may be influenced by another character's account of an event even without having experienced it directly. Their interpretation and emotional response depend on their own personality traits and dispositions. For example, a suspicious character may interpret the account negatively and develop hatred toward the person described; a gullible character may believe the account and also develop hatred; a kind-hearted character may avoid a suspicious or hostile interpretation and may not develop hatred. The same account therefore does not guarantee the same response across characters. The relevant traits, contextual factors, likelihood, and detailed mechanisms remain TBD. |

| D-071 | Changes in interpersonal judgments depend on the personalities of both people involved | Confirmed design direction | A character's response to someone they previously judged negatively may change or persist based on the personality traits and dispositions of both individuals: the character making the judgment and the person being judged. The observer's traits may shape how they interpret later behavior, while the other person's personality may shape the behavior they display and the interaction between them. Positive behavior may lead to a revised opinion or growing affection, but a prior negative judgment may also persist. The specific traits, contextual factors, likelihood, and mechanisms remain TBD. |

| D-072 | Relationships may deteriorate through personality differences and misunderstandings without deliberate intent to harm | Confirmed design direction | Two characters may care for or love one another while differences in personality, misunderstandings, or mistaken interpretations gradually damage their relationship, even when neither character intentionally seeks to hurt the other. Whether this occurs, and how the relationship develops, may depend on the individuals and circumstances. The relevant traits, triggers, likelihood, progression, and possible repair mechanisms remain TBD. |

| D-073 | Damaged relationships may be repaired and potentially become stronger than before | Confirmed design direction | A relationship that has deteriorated may later be repaired. Through time and shared experiences, characters may rebuild trust and closeness, potentially developing a stronger relationship than they had before the damage. Repair and renewed trust are possible, not guaranteed; the relevant personalities, circumstances, actions, time, and conditions remain TBD. |
