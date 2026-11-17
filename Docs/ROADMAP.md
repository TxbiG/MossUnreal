# Roadmap

## Integration foundation (this implementation pass)

- [x] Define the `IMossBackend` boundary and opaque native handles.
- [x] Add runtime backend factory registration and lifecycle manager.
- [x] Add Unreal engine subsystem for process-wide runtime lifecycle.
- [x] Add code-first math, utilities and diagnostics APIs.
- [x] Add JSON-backed data store.
- [x] Add plain C++ system lifecycle/registry.
- [x] Add per-world fixed-step physics subsystem and transform synchronization contract.
- [x] Add static, kinematic and dynamic primitive body adapter configuration.
- [x] Add native raycast contract and Blueprint wrapper.
- [x] Add examples, architecture guides and initial automation tests.
- [ ] Implement and link a concrete provider against the actual Moss public headers/library.
- [ ] Verify native ABI/version and coordinate/unit conversions against the current Moss implementation.
- [ ] Compile with supported Unreal Engine versions and validate on supported platforms.

## Next integration milestones

1. **Native provider:** implement the adapter using only verified Moss APIs; define supported Moss version range and packaging/linkage strategy.
2. **Physics depth:** collision event marshalling, constraints, material mapping, shape assets, overlap queries and debug drawing.
3. **Engine systems:** input/audio/navigation/rendering/resource adapters where supported by Moss.
4. **Editor tooling:** backend selection, diagnostics panel, body inspection and content workflows.
5. **Performance and robustness:** async job scheduling, thread-affinity tests, profiling, stress tests and multi-world tests.
6. **Release validation:** supported engine/platform matrix, CI builds, sample project and versioned release artifacts.
