# Prog4ProjectMathisPfaff

Dig Dug game project made for Programming 4 course DAE.

link: [Prog4ProjectPfaffMathis](https://github.com/MathisPfaff/Prog4ProjectMathisPfaff)

## Features

- Component-based game objects
- Keyboard and controller input support
- State-driven enemy and gameplay behaviour
- Scene management
- Audio system abstraction
- Web build support through Emscripten

## Engine design choices

### GameTime as a singleton
I chose to make `GameTime` a singleton so delta time can be accessed anywhere in the engine without having to thread it through every class or introduce unnecessary dependencies. It keeps timing information easy to reach while avoiding a lot of boilerplate.

### Components as observers through multiple inheritance
Instead of creating separate listener objects for events, I let components inherit from both `BaseComponent` and `Observer`. I liked this approach because the same object that already exists in the engine loop can also react to events, which avoids extra allocations and makes ownership and lifetime management much simpler.

### Dirty flag system
I used dirty flags for data that only needs to be recomputed when something changes. It’s a simple optimization, but one that keeps the engine more efficient.

### Service Locator for global systems
For systems like audio, I chose a `ServiceLocator` approach so gameplay code does not depend directly on a concrete implementation. That makes the rest of the engine less coupled and gives more flexibility to swap or disable systems.

### State pattern for gameplay and enemy behaviour
I used the state pattern for both gameplay flow and enemy behaviour because it keeps behaviour-specific logic contained in dedicated classes instead of turning into large `if`/`switch` structures. This made transitions easier to reason about and made behaviour extensions much cleaner.

### Command-based input
I chose a command-based input system so input handling stays separate from gameplay logic. Instead of hardwiring actions directly to keys or controller buttons, inputs trigger command objects. This makes the system more reusable, easier to remap, and cleaner to support across multiple input devices.

### Multi-phase game loop
I split the engine loop into `FixedUpdate`, `Update`, `LateUpdate`, and `Render` because different types of work belong in different phases.

## Notes

This repository is a project that started from the Minigin engine.
