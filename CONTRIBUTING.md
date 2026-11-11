# Contributing to MossUnreal

Thank you for your interest in contributing to **MossUnreal**.

MossUnreal aims to provide a comprehensive integration between Unreal Engine and the [Moss Framework](https://github.com/TxbiG/Moss).

The project is not limited to physics. Its long-term goal is to make the useful capabilities of Moss accessible to Unreal Engine developers through native Unreal workflows, components, subsystems, resources and editor tooling.

Contributions are welcome across the entire integration, including bug reports, feature requests, documentation, testing, performance improvements and code contributions.

## 1. Project vision

MossUnreal should make it possible for developers to use Moss functionality within Unreal Engine without unnecessarily duplicating the underlying framework.

The integration should cover the following areas as supported by the actual Moss public API:

| Subsystem | Intended integration |
|---|---|
| Physics | 2D/3D simulation, bodies, shapes, collision queries and events |
| Audio | Audio playback, spatial sound, audio resources and controls |
| Input | Input devices, events, mappings, actions and haptic feedback |
| Navigation | 2D/3D navigation, pathfinding and navigation queries |
| Rendering | Rendering capabilities, resource integration and engine-facing rendering utilities |
| Networking | Multiplayer functionality, communication and synchronisation |
| XR | OpenXR and supported VR/AR/MR integration |
| Platform services | Cross-platform functionality and platform-specific backends |
| Diagnostics | Logging, timing, profiling and performance information |
| Other framework systems | Any additional functionality exposed through the Moss public API |

This list describes integration areas, not a promise that every capability is currently available or complete.

## 2. Core principles

All contributions must follow these principles:

- **Moss is the implementation.** Avoid recreating functionality that already belongs in the Moss Framework.
- **Unreal provides the engine integration.** Use Unreal's lifecycle, objects, components, assets, editor and workflow conventions.
- **Use verified public APIs.** Only use functions, types and structures confirmed in the actual Moss public headers.
- **Keep subsystems modular.** Physics, Audio, Input and other systems should be independently usable where practical.
- **Preserve compatibility.** Consider supported engine versions, platforms and configurations.
- **Prioritise performance.** Minimise unnecessary allocations, copies and conversions across the Unreal/Moss boundary.
- **Make ownership explicit.** Document which system owns each native resource.
- **Do not overstate compatibility.** A partially implemented subsystem must not be presented as a complete replacement for Unreal's equivalent.

MossUnreal is an integration layer for the whole framework, not a separate collection of replacement engine systems.

## 3. Before contributing

Before starting work:

1. Read the repository README and architecture documentation.
2. Review the current subsystem support matrix.
3. Check existing issues and pull requests.
4. Confirm the supported Unreal Engine version.
5. Review the relevant Moss public headers.
6. Identify whether the requested functionality belongs in Moss itself or in the Unreal adapter.

Moss source and documentation:

https://github.com/TxbiG/Moss

Never invent Moss functions, classes, configuration options or interfaces. If an API is missing or unclear, document the gap and discuss the appropriate solution before implementing an assumption.

## 4. Development requirements

Depending on the contribution, you may need:

- Git.
- Visual Studio and the appropriate C++ development workload.
- A supported Unreal Engine installation.
- A C++17-compatible compiler.
- CMake 3.20 or newer where required.
- Moss source code and native libraries.
- Platform-specific SDKs for the subsystem being developed.

The README must identify the versions and dependencies required for each supported build configuration.

## 5. Subsystem contribution guidelines

### 5.1 Physics

Physics integration should provide access to Moss's supported 2D and 3D simulation features.

Possible integration areas include:

- Physics worlds and simulation settings.
- Static, dynamic and kinematic bodies where supported.
- Collision shapes and filtering.
- Raycasts and other collision queries.
- Collision events and callbacks.
- Constraints and joints.
- Physics materials.
- Fixed timestep and substep configuration.
- Debug drawing and diagnostics.

Use Unreal-facing components and subsystems where appropriate. Keep the Moss simulation implementation in Moss.

### 5.2 Audio

Audio integration should expose the audio capabilities available through Moss.

Possible areas include:

- Audio device and system initialisation.
- Audio playback and stopping.
- Audio resources and asset loading.
- Volume and playback controls.
- Spatial audio and 3D positioning.
- Audio effects and processing, where supported.
- Audio lifecycle and resource management.
- Device selection and output configuration, where exposed.

Integrations should define how Unreal audio assets and resources relate to Moss audio resources.

Do not duplicate an audio mixer or processing system that already exists in Moss simply to reproduce an Unreal-facing interface.

### 5.3 Input and haptics

Input integration should expose supported Moss input functionality through appropriate Unreal interfaces.

Possible areas include:

- Keyboard and mouse input.
- Game controllers.
- Input device discovery.
- Input states and events.
- Action and mapping integration.
- Device disconnection and reconnection.
- Haptic and vibration feedback.
- Input polling and event timing.

Contributions should avoid generating duplicate input events or conflicting with Unreal's own input processing.

### 5.4 Navigation

Navigation integration should expose the supported Moss 2D and 3D navigation capabilities.

Possible areas include:

- Navigation worlds and configuration.
- Navigation data and surfaces.
- Pathfinding.
- Navigation queries.
- Agent movement and navigation updates.
- Obstacles and avoidance, where supported.
- Navigation debugging and visualisation.

Clearly distinguish Moss navigation data from Unreal navigation data. Conversion or interoperability should be explicit rather than assumed.

### 5.5 Rendering

Rendering contributions should make appropriate Moss rendering functionality accessible to Unreal projects.

Possible areas include:

- Rendering device or backend configuration.
- Graphics resource interoperability.
- Texture and buffer integration.
- Shader and material workflows.
- Rendering utilities.
- Debug rendering.
- Render lifecycle and synchronisation.
- Supported graphics backend selection.

Respect Unreal's render thread, resource ownership and rendering lifecycle. Do not introduce unsafe graphics API access or assume that a Moss graphics resource can be passed directly into Unreal.

### 5.6 Networking and multiplayer

Possible areas include:

- Network initialisation and teardown.
- Connections and disconnections.
- Data transport.
- Message handling.
- Multiplayer communication.
- Synchronisation support.
- Network configuration and diagnostics.

Clearly document how Moss networking relates to Unreal replication and networking. Do not assume the two systems are interchangeable.

### 5.7 XR and platform functionality

Where supported by Moss, contributions may cover:

- OpenXR initialisation.
- VR/AR/MR device integration.
- XR input and tracking.
- Platform-specific backend integration.
- Mobile and desktop platform support.
- Device capabilities and configuration.

Follow Unreal's XR extension and module requirements. Document hardware and platform limitations.

### 5.8 Other framework functionality

Contributions to additional Moss systems are encouraged.

This includes utilities, resource management, timing, logging, profiling, platform abstractions and other facilities that become available through the public API.

New integrations should fit into the existing architecture rather than introduce isolated implementations without a clear reason.

## 6. Architecture and ownership

The intended architecture is:

```text
Unreal Engine
     |
     v
MossUnreal Integration Layer
     |
     +---- Physics Integration
     +---- Audio Integration
     +---- Input Integration
     +---- Navigation Integration
     +---- Rendering Integration
     +---- Networking Integration
     +---- XR Integration
     +---- Platform and Utility Integration
     |
     v
Moss Native Adapter
     |
     v
Moss Framework
```

Keep the engine-facing API separate from the native adapter wherever practical.

Each subsystem should define:

- Initialisation and shutdown.
- Resource creation and destruction.
- Ownership and lifetime.
- Error handling.
- Threading requirements.
- Data and coordinate conversions.
- Supported configurations.
- Known limitations.

Avoid unnecessary dependencies between subsystems. For example, audio integration should not require the physics subsystem unless a specific spatial-audio feature genuinely requires it.

## 7. Code standards

### C++

- Follow the project's existing conventions.
- Use descriptive names.
- Prefer RAII and clear ownership rules.
- Avoid unnecessary raw pointer ownership.
- Validate handles and pointers.
- Minimise allocations in performance-critical code.
- Keep engine-specific implementation separate from Moss adapters.
- Document non-obvious threading and lifecycle decisions.

### Unreal Engine

- Follow Unreal naming and reflection conventions.
- Respect `UObject` and garbage-collection rules.
- Use appropriate modules, subsystems and components.
- Keep `Build.cs` dependencies explicit.
- Respect game, render and audio thread requirements.
- Avoid calling engine APIs from unsupported threads.
- Use Unreal logging and error handling appropriately.

## 8. Branches and commit messages

Create one branch per contribution.

Suggested names:

- `feature/audio-integration`
- `feature/input-integration`
- `feature/navigation-integration`
- `feature/rendering-integration`
- `feature/physics-world`
- `feature/networking`
- `feature/openxr`
- `fix/native-resource-lifecycle`
- `docs/subsystem-guide`
- `test/audio-integration`

Suggested commit messages:

- `feat: add Moss audio subsystem integration`
- `feat: expose Moss input actions`
- `feat: add navigation query support`
- `fix: correct native resource cleanup`
- `docs: document rendering integration`
- `test: add physics lifecycle coverage`

Keep commits focused and avoid mixing unrelated subsystems in one contribution.

## 9. Reporting bugs and proposing features

Bug reports should include the affected subsystem, reproduction steps, expected and actual behaviour, engine version, Moss version or commit, operating system, compiler and relevant logs.

Feature requests should explain:

- The problem being solved.
- The relevant Moss subsystem.
- Why the feature belongs in MossUnreal.
- Whether Moss itself needs changes.
- The expected Unreal-facing workflow.
- Dependencies and performance implications.
- How it can be tested.

Large architectural changes should be discussed before implementation begins.

## 10. Pull requests

Every pull request should:

1. Describe the change and affected subsystem.
2. Reference related issues.
3. Identify the Moss public APIs used.
4. Explain any changes to resource ownership or threading.
5. Include build and testing instructions.
6. Document limitations and unsupported cases.
7. Update the subsystem support matrix.
8. Update relevant documentation and examples.

Before submitting, verify that no unverified Moss APIs have been introduced and that existing integrations have not been unnecessarily broken.

## 11. Testing

Testing should cover the actual behaviour of the subsystem being integrated.

Examples include:

- Physics simulation and collision tests.
- Audio playback and resource lifecycle tests.
- Input event and device tests.
- Haptic feedback tests.
- Navigation and pathfinding tests.
- Rendering resource and synchronisation tests.
- Networking and connection tests.
- XR initialisation and device tests.
- Platform-specific integration tests.
- Cross-subsystem lifecycle tests.

A successful compilation does not prove that an integration is functionally complete.

## 12. Subsystem support and documentation

Maintain a support matrix that classifies each subsystem as:

- **Not started:** no integration available.
- **In development:** work is underway but not ready for general use.
- **Experimental:** usable in limited scenarios with documented limitations.
- **Partial:** some capabilities are implemented and tested.
- **Supported:** documented functionality has passed the required tests.
- **Unsupported:** intentionally not provided by this integration.

Update this matrix whenever functionality is added, removed or changes status.

Do not describe planned functionality as implemented or claim full parity with Unreal Engine.

## 13. Security and licensing

Do not commit credentials, access tokens, private keys or confidential information.

Report security vulnerabilities privately to the repository maintainer.

By contributing, you agree that your contribution may be distributed under this repository's licence. Only submit work that you have permission to redistribute.

## 14. Final note

MossUnreal aims to provide Unreal Engine developers with access to the broader Moss Framework, including physics, audio, input, navigation, rendering, networking, XR and other supported systems.

The priority is a modular, reliable and high-performance integration that preserves Moss as the underlying implementation.

Thank you for contributing to MossUnreal.
