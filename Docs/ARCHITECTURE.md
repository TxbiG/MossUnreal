# Architecture

## Ownership boundaries

- Unreal owns Actors, Components, UObject lifetime and editor-facing configuration.
- Moss owns its simulation world and native physics objects.
- The integration layer owns conversion and mapping between the two object models.

## Planned native adapter

The adapter should be a small, versioned boundary that:
- validates the loaded Moss ABI/version;
- creates and destroys a Moss simulation world;
- advances simulation using an explicit timestep;
- creates/destroys bodies and collision shapes;
- exposes queries and drains collision events;
- reports errors without throwing across the C ABI.

The exact function names and data structures must come from the current public Moss headers.
No guessed Moss symbols should be introduced.

## Simulation policy

The first release should use an explicitly owned Moss world and fixed-step accumulator.
Do not claim that this replaces Chaos globally. It is an opt-in alternative simulation
backend for objects managed by this plugin.
