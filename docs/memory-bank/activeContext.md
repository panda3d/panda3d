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
From `navmesh-tasks.md` Phase 2 (Completed):
1.  [x] **Audit existing AI code**: Check `contrib/src/ai` for integration points.
2.  [x] **Design APIs**: Formalize `NavMeshSettings` and `NavMesh` headers.
3.  [x] **Build System**: Create `panda/src/navmesh/CMakeLists.txt` skeleton.
4.  [x] **Error Handling**: Define policy for generation failures.
5.  [x] **Testing**: Implement unit tests for new module.

## Next Steps
Phase 3: Implementation.
1.  Implement `NavMeshBuilder` logic (geometry extraction + Recast integration).
2.  Implement `NavMesh` logic (Detour integration).
3.  Implement Python bindings.
*   **Build Time**: Adding Recast source might increase build times slightly; need to ensure it's efficient.
*   **Migration**: Users of the old CSV system should not break; we need a clear deprecation or coexistence strategy.
*   **Legacy Code**: Existing `PathFind` class has memory leaks and uses raw pointers. New implementation will be parallel and clean.

