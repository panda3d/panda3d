# Active Context

## Current Focus
We have completed the **Discovery Phase** (Phase 1) and are entering **Refactoring and Stabilization** (Phase 2).
*   **Research**: Validated Recast/Detour as the backend. Confirmed `GeomPrimitive` decomposition strategy for geometry.
*   **Architecture**: Decided on `panda/src/navmesh` location (core module) with optional CMake toggle.
*   **Next Steps**: Prepare the build system and existing code to accept the new module.

## Recent Decisions
*   **Backend**: Use Recast/Detour (MIT license).
*   **Location**: `panda/src/navmesh` (Core) rather than `contrib`.
*   **Geometry**: Use `GeomPrimitive::decompose()` to handle triangle strips and fans uniformly.
*   **API**: Keep `PathFind` class for compatibility but allow it to delegate to the new `NavMesh`.

## Active Tasks
From `navmesh-tasks.md` Phase 2:
1.  [x] **Audit existing AI code**: Check `contrib/src/ai` for integration points (Completed).
2.  [x] **Design APIs**: Formalize `NavMeshSettings` and `NavMesh` headers (Completed).
3.  [x] **Build System**: Create `panda/src/navmesh/CMakeLists.txt` skeleton (Completed).
4.  [ ] **Error Handling**: Define policy for generation failures (warnings vs exceptions).
5.  [ ] **Testing**: Implement unit tests for new module.

## Considerations
*   **Build Time**: Adding Recast source might increase build times slightly; need to ensure it's efficient.
*   **Migration**: Users of the old CSV system should not break; we need a clear deprecation or coexistence strategy.
*   **Legacy Code**: Existing `PathFind` class has memory leaks and uses raw pointers. New implementation will be parallel and clean.

